#include <chrono>
#include <limits>
#include <memory>
#include <vector>

#include <QApplication>
#include <QByteArray>
#include <QTcpServer>
#include <QTcpSocket>
#include <QThread>

#include "FreeAge/common/free_age.hpp"
#include "FreeAge/common/logging.hpp"
#include "FreeAge/common/messages.hpp"
#include "FreeAge/server/game.hpp"
#include "FreeAge/server/match_setup.hpp"
#include "FreeAge/server/settings.hpp"

int main(int argc, char** argv) {
  // Seed the random number generator.
  srand(time(nullptr));
  
  // Initialize loguru.
  loguru::g_preamble_date = false;
  loguru::g_preamble_thread = false;
  loguru::g_preamble_uptime = false;
  loguru::g_stderr_verbosity = 2;
  if (argc > 0) {
    loguru::init(argc, argv, /*verbosity_flag*/ nullptr);
  }
  
  // Create and initialize a QApplication.
  QApplication qapp(argc, argv);
  QCoreApplication::setOrganizationName("FreeAge");
  QCoreApplication::setOrganizationDomain("free-age.org");
  QCoreApplication::setApplicationName("FreeAge");
  
  LOG(INFO) << "Server: Start";
  
  // Parse command line arguments.
  ServerSettings settings;
  settings.serverStartTime = Clock::now();
  if (argc != 2) {
    LOG(INFO) << "Usage: FreeAgeServer <host_token>";
    return 1;
  }
  settings.hostToken = argv[1];
  if (settings.hostToken.size() != hostTokenLength) {
    LOG(ERROR) << "The provided host token has an incorrect length. Required length: " << hostTokenLength << ", actual length: " << settings.hostToken.size();
    return 1;
  }
  
  // Start listening for incoming connections.
  std::shared_ptr<QTcpServer> server(new QTcpServer());
  if (!server->listen(QHostAddress::Any, serverPort)) {
    LOG(ERROR) << "Failed to start listening for connections.";
    return 1;
  }
  
  // Run the main loop for the match setup phase.
  LOG(INFO) << "Server: Entering match setup phase";
  std::vector<std::shared_ptr<PlayerInMatch>> playersInMatch;
  if (!RunMatchSetupLoop(server.get(), &playersInMatch, &settings)) {
    return 0;
  }
  
  // The match has been started.
  LOG(INFO) << "Server: Match starting ...";
  
  // Stop listening for new connections.
  server->close();
  
  // Delete any pending connections.
  while (server->hasPendingConnections()) {
    delete server->nextPendingConnection();
  }
  
  // Drop all players in non-joined state, and convert others to in-game players.
  std::vector<std::shared_ptr<PlayerInGame>> playersInGame;
  for (const auto& player : playersInMatch) {
    if (player->state == PlayerInMatch::State::Joined) {
      std::shared_ptr<PlayerInGame> newPlayer(new PlayerInGame());
      newPlayer->index = playersInGame.size();
      newPlayer->socket = player->socket;
      newPlayer->unparsedBuffer = player->unparsedBuffer;
      newPlayer->name = player->name;
      newPlayer->playerColorIndex = player->playerColorIndex;
      newPlayer->lastPingTime = player->lastPingTime;
      playersInGame.emplace_back(newPlayer);
    }
  }
  
  // Notify all clients about the game start.
  QByteArray msg = CreateStartGameBroadcastMessage();
  for (const auto& player : playersInGame) {
    player->socket->write(msg);
  }
  
  // Reparent all client connections and delete the QTcpServer.
  for (const auto& player : playersInGame) {
    player->socket->setParent(nullptr);
  }
  server.reset();
  
  // Main loop for game loading and game play state
  LOG(INFO) << "Server: Entering game loop";
  RunGameLoop(&playersInGame, &settings);
  
  // Clean up: delete the player sockets.
  for (const auto& player : playersInGame) {
    delete player->socket;
  }
  
  LOG(INFO) << "Server: Exit";
  return 0;
}
