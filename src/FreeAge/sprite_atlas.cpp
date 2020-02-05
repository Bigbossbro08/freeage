#include "FreeAge/sprite_atlas.h"

#include "FreeAge/logging.h"
#include "FreeAge/sprite.h"
#include "RectangleBinPack/MaxRectsBinPack.h"

using namespace rbp;

void SpriteAtlas::AddSprite(Sprite* sprite) {
  sprites.push_back(sprite);
}

QImage SpriteAtlas::BuildAtlas(int width, int height) {
  MaxRectsBinPack packer(width, height, true);
  
  // TODO: Right now, we only pack the sprites' main graphics.
  //       We should pack the shadows too (into another texture, since they are only 8 bit per pixel).
  
  int numRects = 0;
  for (Sprite* sprite : sprites) {
    numRects += sprite->NumFrames();
  }
  
  std::vector<RectSize> rects(numRects);
  int index = 0;
  for (Sprite* sprite : sprites) {
    for (int frameIdx = 0; frameIdx < sprite->NumFrames(); ++ frameIdx) {
      const QImage& image = sprite->frame(frameIdx).graphic.image;
      rects[index] = RectSize{image.width(), image.height()};
      ++ index;
    }
  }
  
  std::vector<Rect> packedRects;
  std::vector<int> packedRectIndices;
  packer.Insert(rects, packedRects, packedRectIndices, MaxRectsBinPack::RectBestShortSideFit);
  if (!rects.empty()) {
    // Not all rects could be added because they did not fit into the specified area.
    return QImage();
  }
  
  // Invert packedRectIndices
  std::vector<int> originalToPackedIndex(numRects);
  for (std::size_t i = 0; i < packedRectIndices.size(); ++ i) {
    originalToPackedIndex[packedRectIndices[i]] = i;
  }
  
  // Draw all images into their assigned rects.
  QImage atlas(width, height, QImage::Format_ARGB32);
  // TODO: Clearing the atlas is only useful to make it look nice. Disable that for release builds?
  //       (However, if we use mip-mapping, some border around each sprite would be useful ...)
  atlas.fill(qRgba(0, 0, 0, 0));
  
  index = 0;
  for (Sprite* sprite : sprites) {
    for (int frameIdx = 0; frameIdx < sprite->NumFrames(); ++ frameIdx) {
      Sprite::Frame::Layer& layer = sprite->frame(frameIdx).graphic;
      const QImage& image = layer.image;
      const Rect& packedRect = packedRects[originalToPackedIndex[index]];
      
      layer.atlasX = packedRect.x;
      layer.atlasY = packedRect.y;
      
      if (packedRect.width == image.width() && packedRect.height == image.height()) {
        layer.rotated = false;
        
        // Draw the image directly into the assigned rect.
        for (int y = 0; y < image.height(); ++ y) {
          for (int x = 0; x < image.width(); ++ x) {
            // TODO: Speed this up with raw access
            atlas.setPixelColor(packedRect.x + x, packedRect.y + y, image.pixelColor(x, y));
          }
        }
      } else if (packedRect.width == image.height() && packedRect.height == image.width()) {
        layer.rotated = true;
        
        // Draw the image into the assigned rect while rotating it by 90 degrees (to the right).
        for (int y = 0; y < image.height(); ++ y) {
          for (int x = 0; x < image.width(); ++ x) {
            // TODO: Speed this up with raw access
            atlas.setPixelColor(packedRect.x + packedRect.width - y, packedRect.y + x, image.pixelColor(x, y));
          }
        }
      } else {
        // Something went wrong.
        LOG(ERROR) << "Internal error of SpriteAtlas::BuildAtlas(): The size of the rect assigned to a sprite frame is incorrect.";
        return QImage();
      }
      
      ++ index;
    }
  }
  return atlas;
}