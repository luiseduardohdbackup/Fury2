/*
SoftFX (Software graphics manipulation library)
Copyright (C) 2003 Kevin Gadd

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2.1 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this library; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

#ifdef _OPENMP
#include <omp.h>
#endif

#include "../header/SoftFX Main.hpp"
#include <string.h>
#include "../header/Debug_Flags.hpp"
#include "../header/SortLL.hpp"
#include "../header/Fury2.hpp"
#include "../header/Blend.hpp"
#include "../header/Clip.hpp"
#include "../header/Resample.hpp"
#include "../header/Blitters.hpp"
#include "../header/Filters.hpp"
#include <math.h>

const float fRadian = 3.14159265358979 / 180.0f;

int ClipFloatLine(Rectangle *Rect, FLine *Line);
int ClipLine(Rectangle *Rect, ILine *Line);
Export int ClipLine(Image *Image, ILine *Line);
Export int ClipFloatLine(Image *Image, FLine *Line);
Export int CheckLineCollide(FRect * rct, FLine * lines, int linecount);
Export int CheckLineCollide2(FRect * rct, SimplePolygon * poly, FLine * lines, int linecount);

Image* ShadowImage = Null;

using namespace std;

Export void SetShadowImage(Image* NewImage) {
    ShadowImage = NewImage;
    return;
}

Image* Tileset::tile(int i) {
  if (this->Initialized) {
    if ((i >= 0) && (i < (int)this->Tiles->size())) {
      return (this->Tiles->at(i));
    }
  }
  return Null;
}


Image* Tileset::tile(int i, short* mapTable) {
  if (!mapTable) return this->tile(i);
  if (this->Initialized) {
    if ((i >= 0) && (i < (int)this->Tiles->size())) {
      int temp = mapTable[i];
      if ((temp >= 0) && (temp < (int)this->Tiles->size())) {
        return (this->Tiles->at(temp));
      }
    }
  }
  return Null;
}

void Tileset::setTile(int i, Image *newTile) {
    if (this->Initialized) {
      if (i < 0) return;
      if (i >= (int)this->Tiles->size()) return;
      if (this->Tiles->at(i)) {
        if (this->Tiles->at(i)->Tags[3] == (DoubleWord)this) {
          delete (this->Tiles->at(i));
          this->Tiles->at(i) = Null;
        } 
      }
      this->Tiles->at(i) = newTile;
    }
}

void Tileset::replaceTile(int i, Image *newTile) {
    if (this->Initialized) {
    if (i < 0) return;
    if (i >= (int)this->Tiles->size()) return;
      if (this->Tiles->at(i)) {
        if (this->Tiles->at(i)->Tags[3] == (DoubleWord)this) {
          delete (this->Tiles->at(i));
          this->Tiles->at(i) = Null;
        } 
      }
      Image* theTile = this->createTile();
      theTile->copy(newTile);
      this->Tiles->at(i) = theTile;
    }
}

Image* Tileset::createTile() {
  Image* iTile = Null;
  iTile = new Image(TileWidth, TileHeight);
  if (iTile) {
    iTile->Tags[3] = (DoubleWord)this;
  }
  RemoveFromHeap(iTile);
  return iTile;
}

void Tileset::addTile(Image *newTile) {
  if (this->Initialized) {
    Image* theTile = this->createTile();
    theTile->copy(newTile);
    this->Tiles->push_back(theTile);
    this->TileCount = this->Tiles->size();
  }
}

void Tileset::addTile(Image *newTile, int i) {
  if (this->Initialized) {
    Image* theTile = this->createTile();
    theTile->copy(newTile);
    this->Tiles->insert(this->Tiles->begin() + i, theTile);
    this->TileCount = this->Tiles->size();
  }
}

void Tileset::removeTile(int i) {
  if (this->Initialized) {
    if (i < 0) return;
    if (i >= (int)this->Tiles->size()) return;
    if (this->Tiles->at(i)) {
      if (this->Tiles->at(i)->Tags[3] == (DoubleWord)this) {
        delete (this->Tiles->at(i));
        this->Tiles->at(i) = Null;
      } 
    }
    std::vector<Image*>::iterator iter = this->Tiles->begin() + i;
    this->Tiles->erase(iter);
    this->TileCount = this->Tiles->size();
  }
}

Export Tileset* AllocateTileset(Image *pTileset, int TileWidth, int TileHeight) {
    return new Tileset(pTileset, TileWidth, TileHeight);
}

Export Tileset* AllocateEmptyTileset(int TileCount, int TileWidth, int TileHeight) {
    return new Tileset(TileCount, TileWidth, TileHeight);
}

Export void PremultiplyTileset(Tileset *pTileset) {
    if (!pTileset) return;
    for (int i = 0; i < pTileset->TileCount; i++) {
      FilterSimple_Premultiply(pTileset->tile(i), Null);
    }
    return;
}

Export void SetTilesetPremultiplied(Tileset *pTileset, int Premultiplied) {
    if (!pTileset) return;
    for (int i = 0; i < pTileset->TileCount; i++) {
      pTileset->tile(i)->OptimizeData.premultiplied = (Premultiplied != 0);
    }
    return;
}

Export void SetTile(Tileset *pTileset, int Index, Image* NewImage) {
    if (!pTileset) return;
    return pTileset->setTile(Index, NewImage);
}

Export void ReplaceTile(Tileset *pTileset, int Index, Image* NewImage) {
    if (!pTileset) return;
    return pTileset->replaceTile(Index, NewImage);
}

Export Image* GetTile(Tileset *pTileset, int Index) {
    if (!pTileset) return 0;
    return pTileset->tile(Index);
}

Export Image* GetTileFast(Tileset *pTileset, unsigned int Index, short* MapTable) {
    if (!pTileset) return 0;
    return pTileset->fastTile(Index, MapTable);
}

Export Image* CreateTile(Tileset *pTileset) {
    if (!pTileset) return 0;
    return pTileset->createTile();
}

Export int GetTileCount(Tileset *pTileset) {
    if (!pTileset) return -1;
    return pTileset->TileCount;
}

Export int GetTileWidth(Tileset *pTileset) {
    if (!pTileset) return -1;
    return pTileset->TileWidth;
}

Export int GetTileHeight(Tileset *pTileset) {
    if (!pTileset) return -1;
    return pTileset->TileHeight;
}

Export void AddTile(Tileset *pTileset, Image* Tile) {
    if (!pTileset) return;
    pTileset->addTile(Tile);
}

Export void InsertTile(Tileset *pTileset, Image* Tile, int Index) {
    if (!pTileset) return;
    pTileset->addTile(Tile, Index);
}

Export void RemoveTile(Tileset *pTileset, int Index) {
    if (!pTileset) return;
    pTileset->removeTile(Index);
}

Export int DeallocateTileset(Tileset *pTileset) {
    if (!pTileset) return Failure;
    delete pTileset;
    return Success;
}

Export int DerefTileset(Tileset *pTileset) {
    if (!pTileset) return Failure;
    pTileset->RefCount--;
    if (pTileset->RefCount < 0) return DeallocateTileset(pTileset);
    return Failure;
}

Export int RefTileset(Tileset *pTileset) {
    if (!pTileset) return Failure;
    return ++pTileset->RefCount;    
}

typedef int TileBlitter(Image *Dest, Image *Source, Rectangle *DestRect, int SourceX, int SourceY, int Opacity);
typedef int TintedTileBlitter(Image *Dest, Image *Source, Rectangle *DestRect, int SourceX, int SourceY, Pixel Tint, int Opacity);

Export int RenderTilemapLayer(TilemapLayerParam *Layer, CameraParam *Camera) {
bool temporaryAnimationMap = false;
int camerax = 0, cameray = 0;
int sx = 0, sy = 0, ex = 0, ey = 0;
int cx = 0, cy = 0;
int dx = 0, dy = 0;
int alpha = 0;
short *pRow, *pTile;
Rectangle rctDest;
Rectangle oldRect;
int maxX = 0, maxY = 0;
int mx = 0, my = 0;
int cv = 0;
TileBlitter* Blitter = Null;
TintedTileBlitter* TintBlitter = Null;
bool yClip = false;
Image *pTarget;
if (!Layer) return Failure;
    if (!Camera) return Failure;
    if (!Camera->pImage()) return Failure;
    if (!Layer->pTileset) return Failure;
    pTarget = Camera->pImage();
    if (Layer->RenderTarget < Camera->RenderTargetCount) {
      pTarget = Camera->pRenderTargets[Layer->RenderTarget];
    }
    pTarget->dirty();
    oldRect = pTarget->ClipRectangle;
    pTarget->ClipRectangle = Camera->Rectangle;
//    Camera->pImage->fill(Pixel(0), &(Camera->Rectangle));

    maxX = Layer->Width - 1;
    maxY = Layer->Height - 1;

    alpha = (Camera->Alpha * Layer->Alpha) / 255;

//    if (!Layer->pAnimationMap) {
//        temporaryAnimationMap = true;
////        Layer->pAnimationMap = AllocateArray(short, Layer->pTileset->TileCount);
////        Layer->pAnimationMap = LookupAllocate<short>(Layer->pTileset->TileCount);
//        Layer->pAnimationMap = StaticAllocate<short>(MappingBuffer, Layer->pTileset->TileCount);
//        for (int i = 0; i < Layer->pTileset->TileCount; i++) {
//          Layer->pAnimationMap[i] = i;
//        }
//    }

    camerax = Camera->ViewportX;
    cameray = Camera->ViewportY;

    sx = Layer->X1;
    sy = Layer->Y1;
    mx = Layer->X2 - Layer->X1;
    my = Layer->Y2 - Layer->Y1;
    if (mx <= 0) mx = (Camera->Rectangle.Width / Layer->pTileset->TileWidth) + 2;
    if (my <= 0) my = (Camera->Rectangle.Height / Layer->pTileset->TileHeight) + 2;

    // if the camera value is above one tile, we can skip those tiles entirely and only offset by the remainder
    if (camerax > 0) {
        sx += (camerax / Layer->pTileset->TileWidth);
        camerax = (camerax % Layer->pTileset->TileWidth);
    }
    if (cameray > 0) { 
        sy += (cameray / Layer->pTileset->TileHeight);
        cameray = (cameray % Layer->pTileset->TileHeight);
    }

    // clip the start and end tile values so we don't try and draw stuff that isn't there
    if (Layer->WrapX) {
		    ex = sx + (Camera->Rectangle.Width / Layer->pTileset->TileWidth) + 2;
    } else {
        ex = sx + ClipValue(ClipValue(mx, (Camera->Rectangle.Width / Layer->pTileset->TileWidth) + 2), Layer->Width);
    }
    if (Layer->WrapY) {
		    ey = sy + (Camera->Rectangle.Height / Layer->pTileset->TileHeight) + 2;
    } else {
        ey = sy + ClipValue(ClipValue(my, (Camera->Rectangle.Height / Layer->pTileset->TileHeight) + 2), Layer->Height);
    }

    // some final clipping
    if (Layer->WrapX) {
        ex = ClipValue(ex, sx, ex);
    } else {
        sx = ClipValue(sx, Layer->X1, Layer->Width);
        ex = ClipValue(ex, sx, Layer->Width);
    }
    if (Layer->WrapY) {
        ey = ClipValue(ey, sy, ey);
    } else {
        sy = ClipValue(sy, Layer->Y1, Layer->Height);
        ey = ClipValue(ey, sy, Layer->Height);
    }

    switch(Layer->Effect) {
    default:
    case 0:
        Blitter = BlitSimple_Normal_Opacity;
        TintBlitter = BlitSimple_Normal_Tint_Opacity;
        break;
    case 1:
        Blitter = BlitSimple_Automatic_Matte_Opacity;
        TintBlitter = BlitSimple_Matte_Tint_Opacity;
        break;
    case 2:
        Blitter = BlitSimple_Automatic_SourceAlpha_Opacity;
        TintBlitter = BlitSimple_SourceAlpha_Tint_Opacity;
        break;
    case 3:
        Blitter = BlitSimple_Additive_Opacity;
        break;
    case 4:
        Blitter = BlitSimple_Subtractive_Opacity;
        break;
    case 6:
        Blitter = BlitSimple_Screen_Opacity;
        break;
    case 7:
        Blitter = BlitSimple_Multiply_Opacity;
        break;
    case 8:
        Blitter = BlitSimple_Lightmap_Opacity;
        break;
    }

    rctDest.Width = Layer->pTileset->TileWidth;
    rctDest.Height = Layer->pTileset->TileHeight;

    if (Layer->TintColor[::Alpha]) {
      if (TintBlitter == Null) {
        enableClipping = true;
        pTarget->ClipRectangle = oldRect;
        return Failure;
      }
    } else {
      if (Blitter == Null) {
        enableClipping = true;
        pTarget->ClipRectangle = oldRect;
        return Failure;
      }
    }

    int result;
    if (result = Override::EnumOverrides(Override::RenderTilemapLayer, 9, Layer, Camera, sx, sy, ex, ey, camerax, cameray, alpha)) {
      if (temporaryAnimationMap) {
        Layer->pAnimationMap = Null;
      }
      enableClipping = true;
      pTarget->ClipRectangle = oldRect;
      return result;
    }

    // initialize the y coordinate
    dy = -cameray;
    for (cy = sy; cy < ey; ++cy) {
        rctDest.Top = dy;
        dx = -camerax;
        if (Layer->WrapY) {
            pRow = Layer->pData + (Layer->Width * (cy % maxY));
        } else {
            pRow = Layer->pData + (Layer->Width * cy);
        }
        yClip = (dy >= pTarget->ClipRectangle.Top) && ((dy + Layer->pTileset->TileHeight) < pTarget->ClipRectangle.bottom());
        pTile = pRow + sx;
        if (Layer->pAnimationMap) {
          if (Layer->TintColor[::Alpha]) {
            for (cx = sx; cx < ex; ++cx) {
                rctDest.Left = dx;
                if (Layer->WrapX) {
                    pTile = pRow + (cx % maxX);
                }
                cv = Layer->pAnimationMap[*pTile];
                if ((cv == Layer->MaskedTile) || (cv >= Layer->pTileset->TileCount) || (cv < 0)) {
                } else {
                    enableClipping = !((dx >= pTarget->ClipRectangle.Left) && ((dx + Layer->pTileset->TileWidth) < pTarget->ClipRectangle.right()) && yClip);
                    TintBlitter(pTarget, Layer->pTileset->tile(cv), &rctDest, 0, 0, Layer->TintColor, alpha);
                }
                dx += Layer->pTileset->TileWidth;
                pTile++;
            }
          } else {
            for (int cx = sx; cx < ex; ++cx) {
                rctDest.Left = dx;
                if (Layer->WrapX) {
                    pTile = pRow + (cx % maxX);
                }
                cv = Layer->pAnimationMap[*pTile];
                if ((cv == Layer->MaskedTile) || (cv >= Layer->pTileset->TileCount) || (cv < 0)) {
                } else {
                    enableClipping = !((dx >= pTarget->ClipRectangle.Left) && ((dx + Layer->pTileset->TileWidth) < pTarget->ClipRectangle.right()) && yClip);
                    Blitter(pTarget, Layer->pTileset->tile(cv), &rctDest, 0, 0, alpha);
                }
                dx += Layer->pTileset->TileWidth;
                pTile++;
            }
          }
        } else {
          if (Layer->TintColor[::Alpha]) {
            for (int cx = sx; cx < ex; ++cx) {
                rctDest.Left = dx;
                if (Layer->WrapX) {
                    pTile = pRow + (cx % maxX);
                }
                cv = *pTile;
                if ((cv == Layer->MaskedTile) || (cv >= Layer->pTileset->TileCount) || (cv < 0)) {
                } else {
                    enableClipping = !((dx >= pTarget->ClipRectangle.Left) && ((dx + Layer->pTileset->TileWidth) < pTarget->ClipRectangle.right()) && yClip);
                    TintBlitter(pTarget, Layer->pTileset->tile(cv), &rctDest, 0, 0, Layer->TintColor, alpha);
                }
                dx += Layer->pTileset->TileWidth;
                pTile++;
            }
          } else {
            for (int cx = sx; cx < ex; ++cx) {
                rctDest.Left = dx;
                if (Layer->WrapX) {
                    pTile = pRow + (cx % maxX);
                }
                cv = *pTile;
                if ((cv == Layer->MaskedTile) || (cv >= Layer->pTileset->TileCount) || (cv < 0)) {
                } else {
                    enableClipping = !((dx >= pTarget->ClipRectangle.Left) && ((dx + Layer->pTileset->TileWidth) < pTarget->ClipRectangle.right()) && yClip);
                    Blitter(pTarget, Layer->pTileset->tile(cv), &rctDest, 0, 0, alpha);
                }
                dx += Layer->pTileset->TileWidth;
                pTile++;
            }
          }
        }
        dy += Layer->pTileset->TileHeight;
    }
    if (temporaryAnimationMap) {
//        DeleteArray(Layer->pAnimationMap);
//        LookupDeallocate(Layer->pAnimationMap);
      Layer->pAnimationMap = Null;
    }
    enableClipping = true;
    pTarget->ClipRectangle = oldRect;
    return Success;
}

Export int FillSpriteMatrix(SpriteParam *List, CollisionMatrix *Matrix) {
SpriteParam *current = List;
  if (!current) return Failure;
  if (!Matrix) return Failure;
  return Success;
  while (current) {
    Matrix->addSprite(current);
    current = current->pNext;
  }
  return Success;
}

int CollisionCheckEx(SpriteParam *first, SpriteParam *check, bool mustbesolid, int requiredtype, int excludedtype, SpriteParam **out) {
SpriteParam * sCurrent = first;
FRect rSource, rDest;
    if (!first) return 0;
    if (!check) return 0;
    if (!first->pNext) return 0;
    while (sCurrent) {
        if (sCurrent != check) {
            if ((sCurrent->Type != excludedtype) && (((requiredtype >= 0) && (sCurrent->Type == requiredtype)) || (requiredtype < 0))) {
                if ((sCurrent->Stats.Solid) || (!mustbesolid)) {
                    if (check->touches(sCurrent)) {
                      if (out) *out = sCurrent;
                      return sCurrent->Index;
                    }
                }
            }
        }
nevar:
        sCurrent = sCurrent->pNext;
    }
    return 0;
}

int CollisionCheck3(SpriteParam *first, SpriteParam *check, bool mustbesolid, int requiredtype, int excludedtype, SpriteParam **out) {
    if (!first) return 0;
    if (!check) return 0;
FRect rSource = check->getRect();
ListSpriteIterator iter(first);
    SpriteParam* sCurrent = iter.current();
    while (sCurrent) {
        if (sCurrent != check) {
            if ((sCurrent->Type != excludedtype) && (((requiredtype >= 0) && (sCurrent->Type == requiredtype)) || (requiredtype < 0))) {
                if ((sCurrent->Stats.Solid) || (!mustbesolid)) {
                    if (check->touches(sCurrent)) {
                      if (out) *out = sCurrent;
                      return sCurrent->Index;
                    }
                }
            }
        }
nevar:
        sCurrent = iter.next();
    }
    return 0;
}

Export int CollisionCheck(SpriteParam *first, SpriteParam *check, bool mustbesolid, int requiredtype, int excludedtype) {
  return CollisionCheckEx(first, check, mustbesolid, requiredtype, excludedtype, Null);
}

#define _check iCollide = -Sprite->touches(Matrix); \
    if (iCollide == 0) { \
      iCollide = CollisionCheck3(List, Sprite, true, -1, -1, Null); \
    }

Export bool ResolveCollisions(SpriteParam *List, SpriteParam *Sprite, VelocityVector &ResolvedSpeed, CollisionMatrix *Matrix) {
SpritePosition spOldPosition = Sprite->Position;
int iCollide = 0;
float fX = ResolvedSpeed.X, fY = ResolvedSpeed.Y;
bool resolved = false;
    resolved = false;
    Sprite->Position = spOldPosition;
    Sprite->Position.X += ResolvedSpeed.X;
    Sprite->Position.Y += ResolvedSpeed.Y;
    _check;
    if (iCollide == 0) {
        Sprite->Position = spOldPosition;
        return false;
    } else {
    }
    resolved = false;
    if (abs(fX) > 0.1) {
      while (iCollide) {
          if (iCollide > 0) {
              Sprite->Events.CollidedWith = iCollide;
          } else {
              Sprite->Events.CollidedWithMap = true;
          }
          Sprite->Position = spOldPosition;
          Sprite->Position.X += fX;
          Sprite->Position.Y += fY;
          _check;
          if (iCollide == 0) {
              resolved = true; 
              break;
          }
          fX /= 2;
          if (fX == 0) {
            fX = ResolvedSpeed.X;
            break;
          }
          if ((abs(fX) < 1)) {
              fX = 0;
          }
      }
      if (resolved) {
          Sprite->Position = spOldPosition;
          ResolvedSpeed.X = fX;
          ResolvedSpeed.Y = fY;
          return true;
      }
    }

    fX = ResolvedSpeed.X; fY = ResolvedSpeed.Y;

    resolved = false;
    if (abs(fY) > 0.1) {
      while (iCollide) {
          if (iCollide > 0) {
              Sprite->Events.CollidedWith = iCollide;
          } else {
              Sprite->Events.CollidedWithMap = true;
          }
          Sprite->Position = spOldPosition;
          Sprite->Position.X += fX;
          Sprite->Position.Y += fY;
          _check;
          if (iCollide == 0) {
              resolved = true; 
              break;
          }
          fY /= 2;
          if (fY == 0) {
            fY = ResolvedSpeed.Y;
            break;
          }
          if ((abs(fY) < 1)) {
              fY = 0;
          }
      }
      if (resolved) {
          Sprite->Position = spOldPosition;
          ResolvedSpeed.X = fX;
          ResolvedSpeed.Y = fY;
          return true;
      }
    }

    fX = ResolvedSpeed.X; fY = ResolvedSpeed.Y;

    resolved = false;
    if ((abs(fX) > 0.1) || (abs(fY) > 0.1)) {
      while (iCollide) {
          if (iCollide > 0) {
              Sprite->Events.CollidedWith = iCollide;
          } else {
              Sprite->Events.CollidedWithMap = true;
          }
          Sprite->Position = spOldPosition;
          Sprite->Position.X += fX;
          Sprite->Position.Y += fY;
          _check;
          if (iCollide == 0) {
              resolved = true; 
              break;
          }
          fX /= 2;
          fY /= 2;
          if ((fX == 0) && (fY == 0)) break;
          if ((abs(fX) < 1)) {
              fX = 0;
          }
          if ((abs(fY) < 1)) {
              fY = 0;
          }
      }
      if (resolved) {
          ResolvedSpeed.X = fX;
          ResolvedSpeed.Y = fY;
      } else {
          ResolvedSpeed.X = 0;
          ResolvedSpeed.Y = 0;
      }
    }

    Sprite->Position = spOldPosition;
    Sprite->Position.X += ResolvedSpeed.X;
    Sprite->Position.Y += ResolvedSpeed.Y;
    _check;
    if (iCollide == 0) {
    } else {
      ResolvedSpeed.X = 0;
      ResolvedSpeed.Y = 0;
    }

    Sprite->Position = spOldPosition;
    return true;
}

#undef _check

Export int UpdateSprites(SpriteParam *List, SpriteEngineOptions *Options) {
SpriteParam *pCurrent = List, *pCheck;
VelocityVector vSpeed, vCheck;
int iCount = 0;
bool bCollided = false;
std::vector<ForceEntry> forceEntries;
std::vector<ForceEntry>::iterator currentEntry;
ForceEntry newEntry;
    if (!List) return Failure;
    if (!Options) return Failure;
    if (!Options->Matrix) return Failure;
    //FillSpriteMatrix(List, Options->Matrix);
    // reset
    while (pCurrent) {
        pCurrent->Events.CollidedWith = 0;
        pCurrent->Events.CollidedWithMap = false;
        pCurrent->Events.FadedOut = false;
        pCurrent->Events.Changed = false;
        pCurrent->Events.Moved = false;
        pCurrent->Velocity.XF = pCurrent->Velocity.CXF;
        pCurrent->Velocity.YF = pCurrent->Velocity.CYF;
        if (Options->VelocityMultiplier != 1) {
          float cfm = pow(pCurrent->Velocity.CFM, Options->VelocityMultiplier);
          pCurrent->Velocity.CXF *= cfm;
          pCurrent->Velocity.CYF *= cfm;
        } else {
          pCurrent->Velocity.CXF *= pCurrent->Velocity.CFM;
          pCurrent->Velocity.CYF *= pCurrent->Velocity.CFM;
        }
        pCurrent->Velocity.FW = 0;
        pCurrent = pCurrent->pNext;
    }
    // calculate forces
    bool restart = true;
    forceEntries = std::vector<ForceEntry>();
    _DebugTrace("beginning physics loop");
    while (restart) {
      _DebugTrace("iterating\n");
      restart = false;
      pCurrent = List;
      while (pCurrent) {
        SpritePosition oldposition;
        SpritePosition oldcheckposition;
        float forcemultiplier;
        if (!(pCurrent->Stats.Cull && pCurrent->Culled)) {
          if ((pCurrent->Reserved1 == 0) || (pCurrent->Stats.CanPush) || ((pCurrent->Stats.Solid) && ((pCurrent->Velocity.XF != 0) || (pCurrent->Velocity.YF != 0)))) {
            pCurrent->Reserved1 = 1;
            vSpeed.X = pCurrent->Velocity.X + (sin(pCurrent->Velocity.B * fRadian) * pCurrent->Velocity.V) + pCurrent->Velocity.XF;
            vSpeed.Y = pCurrent->Velocity.Y + (-cos(pCurrent->Velocity.B * fRadian) * pCurrent->Velocity.V) + pCurrent->Velocity.YF;
            vSpeed.X *= pCurrent->Velocity.getVM(Options->VelocityMultiplier);
            vSpeed.Y *= pCurrent->Velocity.getVM(Options->VelocityMultiplier);

            oldposition = pCurrent->Position;
            pCurrent->Position.X += vSpeed.X;
            pCurrent->Position.Y += vSpeed.Y;
            pCheck = List;
            currentEntry = forceEntries.end();
            for (std::vector<ForceEntry>::iterator scan_iter = forceEntries.begin(); scan_iter != forceEntries.end(); ++scan_iter) {
              if (scan_iter->Sprite == pCurrent) {
                currentEntry = scan_iter;
                break;
              }
            }
            if (currentEntry == forceEntries.end()) {
              newEntry.Sprite = pCurrent;
              newEntry.Items.clear();
              forceEntries.push_back(newEntry);
              currentEntry = forceEntries.end();
              currentEntry--;
            }
            while (pCheck) {
              if (pCheck != pCurrent) {
                if (pCheck->Stats.Solid) {
                  if (pCheck->Stats.Pushable) {
                    vCheck.X = pCheck->Velocity.X + (sin(pCheck->Velocity.B * fRadian) * pCheck->Velocity.V) + pCheck->Velocity.XF;
                    vCheck.Y = pCheck->Velocity.Y + (-cos(pCheck->Velocity.B * fRadian) * pCheck->Velocity.V) + pCheck->Velocity.YF;
                    vCheck.X *= pCheck->Velocity.getVM(Options->VelocityMultiplier);
                    vCheck.Y *= pCheck->Velocity.getVM(Options->VelocityMultiplier);

                    oldcheckposition = pCheck->Position;
                    pCheck->Position.X += vCheck.X;
                    pCheck->Position.Y += vCheck.Y;
                    if (pCurrent->touches(pCheck)) {
                      std::list<ForceEntry>::iterator iter = currentEntry->Items.begin();
                      bool alreadyProcessed = false;
                      while (iter != currentEntry->Items.end()) {
                        if (iter->Sprite == pCheck) {
                          alreadyProcessed = true;
                          break;
                        }
                        iter++;
                      }
                      if (!alreadyProcessed) {
                        newEntry.Sprite = pCheck;
                        newEntry.Items.clear();
                        currentEntry->Items.push_back(newEntry);
                        forcemultiplier = 1;
                        if (pCheck->Stats.Weight > 0) {
                          forcemultiplier = pCurrent->Stats.Weight / pCheck->Stats.Weight;
                          if (forcemultiplier > 1) forcemultiplier = 1;
                          if (forcemultiplier < 0) forcemultiplier = 0;
                        }
                        pCheck->Velocity.XF += (vSpeed.X * forcemultiplier);
                        pCheck->Velocity.YF += (vSpeed.Y * forcemultiplier);
                        pCheck->Velocity.FW += (pCurrent->Stats.Weight * forcemultiplier);
                        pCheck->Reserved1 = 0;
                        restart = true;
                        _DebugTrace("sprite pushed\n");
                      }
                    }
                    pCheck->Position = oldcheckposition;
                  }
                }
              }
              pCheck = pCheck->pNext;
            }
            pCurrent->Position = oldposition;
          }
          pCurrent->Events.Moved = false;
          pCurrent = pCurrent->pNext;
        }
      }
    }
    forceEntries.clear();
    pCurrent = List;
    while (pCurrent) {
        iCount++;
        
        if (!(pCurrent->Stats.Cull && pCurrent->Culled)) {

            vSpeed.X = pCurrent->Velocity.X + (sin(pCurrent->Velocity.B * fRadian) * pCurrent->Velocity.V) + pCurrent->Velocity.XF;
            vSpeed.Y = pCurrent->Velocity.Y + (-cos(pCurrent->Velocity.B * fRadian) * pCurrent->Velocity.V) + pCurrent->Velocity.YF;

            vSpeed.X *= pCurrent->Velocity.getVM(Options->VelocityMultiplier);
            vSpeed.Y *= pCurrent->Velocity.getVM(Options->VelocityMultiplier);

            pCurrent->Events.Moved = ((abs(vSpeed.X) > 0) || (abs(vSpeed.Y) > 0));

            if (pCurrent->Stats.Solid) {
              bCollided = ResolveCollisions(List, pCurrent, vSpeed, Options->Matrix);
            } else {
              bCollided = false;
            }

            pCurrent->Events.Changed = ((abs(pCurrent->Velocity.A) > 0) || (abs(pCurrent->Velocity.BR) > 0));
            pCurrent->Position.X += vSpeed.X;
            pCurrent->Position.Y += vSpeed.Y;
            pCurrent->Position.Z += pCurrent->Velocity.Z * pCurrent->Velocity.getVM(Options->VelocityMultiplier);

            if (pCurrent->Velocity.A != 0) {
                pCurrent->Params.Alpha += pCurrent->Velocity.A * pCurrent->Velocity.getVM(Options->VelocityMultiplier);
                if (pCurrent->Velocity.A < 0) {
                    if (pCurrent->Params.Alpha <= pCurrent->Velocity.AT) {
                        pCurrent->Params.Alpha = pCurrent->Velocity.AT;
                        pCurrent->Velocity.A = 0;
                        pCurrent->Events.FadedOut = true;
                    }
                } else if (pCurrent->Velocity.A > 0) {
                    if (pCurrent->Params.Alpha >= pCurrent->Velocity.AT) {
                        pCurrent->Params.Alpha = pCurrent->Velocity.AT;
                        pCurrent->Velocity.A = 0;
                        pCurrent->Events.FadedOut = true;
                    }
                }
            }

            if (pCurrent->Velocity.BR != 0) {
                pCurrent->Velocity.B += pCurrent->Velocity.BR * pCurrent->Velocity.getVM(Options->VelocityMultiplier);
                if (pCurrent->Velocity.BR < 0) {
                    if (pCurrent->Velocity.B <= pCurrent->Velocity.BRT) {
                        pCurrent->Velocity.B = pCurrent->Velocity.BRT;
                        pCurrent->Velocity.BR = 0;                    }
                } else if (pCurrent->Velocity.BR > 0) {
                    if (pCurrent->Velocity.B >= pCurrent->Velocity.BRT) {
                        pCurrent->Velocity.B = pCurrent->Velocity.BRT;
                        pCurrent->Velocity.BR = 0;
                    }
                }
            }

        }

        pCurrent = pCurrent->pNext;
    }
    FillSpriteMatrix(List, Options->Matrix);
    return Success;
}

Export int CullSprites(SpriteParam *List, CameraParam *Camera) {
SpriteParam *pCurrent = List;
Rectangle rctDest, rctSource, rctCopy;
float x = 0, y = 0, w = 0, h = 0;
int iCount = 0;
    if (!List) return Failure;
    if (!Camera) return Failure;
    while (pCurrent) {
        iCount++;
        if (pCurrent->Params.Alpha != 0) {
            if (pCurrent->Graphic.pImage) {
                pCurrent->Culled = true;
                w = pCurrent->Graphic.Rectangle.Width;
                h = pCurrent->Graphic.Rectangle.Height;
                x = (pCurrent->Position.X * Camera->ParallaxX) - Camera->ViewportX - (pCurrent->Graphic.XCenter - (w / 2));
                y = (pCurrent->Position.Y * Camera->ParallaxY) - Camera->ViewportY + (h) - pCurrent->Graphic.YCenter;
                if (pCurrent->Params.Scale == 1) {
                    w /= 2;
                    if ((y) < Camera->Rectangle.Top) goto nextsprite;
                    if ((y - h) > Camera->Rectangle.bottom()) goto nextsprite;
                    if ((x + w) < Camera->Rectangle.Left) goto nextsprite;
                    if ((x - w) > Camera->Rectangle.right()) goto nextsprite;
                    rctDest.Left = ceil(x - (w));
                    rctDest.Top = ceil(y - h);
                    rctDest.Width = pCurrent->Graphic.Rectangle.Width;
                    rctDest.Height = pCurrent->Graphic.Rectangle.Height;
                    rctSource = pCurrent->Graphic.Rectangle;
                    if (Clip2D_PairToRect(&rctDest, &rctSource, &(Camera->Rectangle))) {
                        pCurrent->Culled = false;
                    }
                } else if (pCurrent->Params.Scale != 0) {
                    w *= abs(pCurrent->Params.Scale) / 2;
                    if ((y) < Camera->Rectangle.Top) goto nextsprite;
                    if ((y - h) > Camera->Rectangle.bottom()) goto nextsprite;
                    if ((x + w) < Camera->Rectangle.Left) goto nextsprite;
                    if ((x - w) > Camera->Rectangle.right()) goto nextsprite;
                    rctDest.Left = ceil(x - (w));
                    rctDest.Top = ceil(y - (h * pCurrent->Params.Scale));
                    rctDest.Width = pCurrent->Graphic.Rectangle.Width * abs(pCurrent->Params.Scale);
                    rctDest.Height = pCurrent->Graphic.Rectangle.Height * pCurrent->Params.Scale;
                    rctSource = pCurrent->Graphic.Rectangle;
                    rctCopy = rctDest;
                    if (ClipRectangle_Rect(&rctCopy, &(Camera->Rectangle))) {
                        pCurrent->Culled = false;
                    }
                }
            }
        }
nextsprite:        
        pCurrent = pCurrent->pNext;
    }
    return iCount;
}

Export SpriteParam* SortSprites(SpriteParam *List) {
SpriteParam *result = Null;
  result = SortLinkedList<SpriteParam>(List);
  return result;
}

Export int RenderSprites(SpriteParam *Start, CameraParam *Camera, RenderSpritesParam *Options) {
SpriteParam *pCurrent = Start;
Rectangle rctDest, rctSource, rctCopy;
Image *pImage, *pTarget;
float x = 0, y = 0, w = 0, h = 0;
float s = 0, r = 0;
float px[4], py[4];
Polygon<TexturedVertex> poly;
RenderFunction *renderer = Null;
bool scaled = false, rotated = false;
int iCount = 0, iTemp = 0;
int iSecondaryImage = -1;
Image* currentImage = 0;
Pixel white = Pixel(255,255,255,255);
    if (!Start) return Failure;
    if (!Camera) return Failure;
    if (!Camera->pImage()) return Failure;
    if (!Options) return Failure;
    poly.Allocate(4);
    while (pCurrent) {
      if ((pCurrent->Visible) && (!(pCurrent->Culled))) {
        pTarget = Camera->pImage();
        if (pCurrent->Params.RenderTarget < Camera->RenderTargetCount) {
          pTarget = Camera->pRenderTargets[pCurrent->Params.RenderTarget];
        }
        if ((pCurrent->Params.Alpha != 0)) {
          iCount++;
          switch  (pCurrent->Params.SpecialFX) {
          default:
          case 0:
            break;
          case fxHardShadow:
            break;
          case fxSoftShadow:
            if (Options->ShadowImage) {
              Image* shadowImage = (Image*)Options->ShadowImage;
              rctDest = pCurrent->getRectangle();
              rctSource = shadowImage->getRectangle();
              BlitResample_Subtractive_Opacity(pTarget, shadowImage, &rctDest, &rctSource, DefaultSampleFunction, pCurrent->Params.Alpha * Camera->Alpha);
            }
            break;
          case fxCastShadow:
            break;
          }
          iSecondaryImage = -1;
          while (iSecondaryImage < pCurrent->Graphic.SecondaryImageCount) {
            if (iSecondaryImage >= 0) {
              if (Options->DrawSecondaryImages) {
                switch (pCurrent->Graphic.pSecondaryImages[iSecondaryImage].ImageType) {
                  case siOverlay:
                    currentImage = pCurrent->Graphic.pSecondaryImages[iSecondaryImage].pImage;
                  default:
                    currentImage = 0;
                    break;
                }
              } else {
                currentImage = 0;
              }
            } else {
              if (Options->DrawFrames) {
                currentImage = pCurrent->Graphic.pImage;
              } else {
                currentImage = 0;
              }
            }
            iSecondaryImage++;
            if (currentImage) {
              w = pCurrent->Graphic.Rectangle.Width;
              h = pCurrent->Graphic.Rectangle.Height;
              x = (pCurrent->Position.X * Camera->ParallaxX) - Camera->ViewportX - (pCurrent->Graphic.XCenter - (w/2));
              y = (pCurrent->Position.Y * Camera->ParallaxY) - Camera->ViewportY + (h) - pCurrent->Graphic.YCenter;
              s = pCurrent->Params.Scale;
              r = pCurrent->Params.Angle;
              scaled = (s != 1);
              rotated = (((int)r) % 360) != 0;
              if ((!scaled) && (!rotated) && (!(pCurrent->Params.Beam))) {
                w /= 2;
                if ((y) < Camera->Rectangle.Top) goto nextsprite;
                if ((y - h) > Camera->Rectangle.bottom()) goto nextsprite;
                if ((x + w) < Camera->Rectangle.Left) goto nextsprite;
                if ((x - w) > Camera->Rectangle.right()) goto nextsprite;
                rctDest.Left = ceil(x - (w));
                rctDest.Top = ceil(y - h);
                rctDest.Width = pCurrent->Graphic.Rectangle.Width;
                rctDest.Height = pCurrent->Graphic.Rectangle.Height;
                rctSource = pCurrent->Graphic.Rectangle;
                if (Clip2D_PairToRect(&rctDest, &rctSource, &(Camera->Rectangle))) {
                    switch(pCurrent->Params.BlitMode) {
                    default:
                    case 0:
                        iTemp = currentImage->MatteColor.V;
                        currentImage->MatteColor = pCurrent->Graphic.MaskColor;
                        if (pCurrent->Params.Color[::Alpha] > 0) {
                          BlitSimple_Matte_Tint_Opacity(pTarget, currentImage, &rctDest, rctSource.Left, rctSource.Top, pCurrent->Params.Color, abs(pCurrent->Params.Alpha) * Camera->Alpha);
                        } else {
                          BlitSimple_Matte_Opacity(pTarget, currentImage, &rctDest, rctSource.Left, rctSource.Top, abs(pCurrent->Params.Alpha) * Camera->Alpha);
                        }
                        currentImage->MatteColor.V = iTemp;
                        break;
                    case 1:
                        if (pCurrent->Params.Color[::Alpha] > 0) {
                          BlitSimple_SourceAlpha_Tint_Opacity(pTarget, currentImage, &rctDest, rctSource.Left, rctSource.Top, pCurrent->Params.Color, abs(pCurrent->Params.Alpha) * Camera->Alpha);
                        } else {
                          BlitSimple_Automatic_SourceAlpha_Opacity(pTarget, currentImage, &rctDest, rctSource.Left, rctSource.Top, abs(pCurrent->Params.Alpha) * Camera->Alpha);
                        }
                        break;
                    case 2:
                        BlitSimple_Additive_Opacity(pTarget, currentImage, &rctDest, rctSource.Left, rctSource.Top, abs(pCurrent->Params.Alpha) * Camera->Alpha);
                        break;
                    case 3:
                        BlitSimple_Subtractive_Opacity(pTarget, currentImage, &rctDest, rctSource.Left, rctSource.Top, abs(pCurrent->Params.Alpha) * Camera->Alpha);
                        break;
                    case 4:
                        // Gamma
                        break;
                    case 5:
                        BlitSimple_Screen_Opacity(pTarget, currentImage, &rctDest, rctSource.Left, rctSource.Top, abs(pCurrent->Params.Alpha) * Camera->Alpha);
                        break;
                    case 6:
                        BlitSimple_Multiply_Opacity(pTarget, currentImage, &rctDest, rctSource.Left, rctSource.Top, abs(pCurrent->Params.Alpha) * Camera->Alpha);
                        break;
                    case 7:
  //                      BlitSimple_Lightmap_RGB_Opacity(pTarget, currentImage, &rctDest, rctSource.Left, rctSource.Top, abs(pCurrent->Params.Alpha) * Camera->Alpha);
                        // Lightmap
                        break;
                    case 8:
                        BlitSimple_Merge_Opacity(pTarget, currentImage, &rctDest, rctSource.Left, rctSource.Top, abs(pCurrent->Params.Alpha) * Camera->Alpha);
                        break;
                    }
                    if (Options->DrawFrameRectangles) {
                      Rectangle rctOutline = rctDest;
                      FilterSimple_Box(pTarget, &rctOutline, Options->FrameRectangleColor);
                    }
                    if (Options->DrawSortingLines) {
                      FilterSimple_Line_AA(pTarget, rctDest.Left, rctDest.bottom_exclusive() + pCurrent->ZLeft, rctDest.right_exclusive(), rctDest.bottom_exclusive() + pCurrent->ZRight, Options->SortingLineColor);
                    }
                }
              } else {
                if (rotated || (pCurrent->Params.Beam)) {
                  switch(pCurrent->Params.BlitMode) {
                  default:
                    renderer = Null;
                    break;
                  case 1:
                    renderer = RenderFunction_SourceAlpha;
                    break;
                  case 2:
                    renderer = RenderFunction_Additive;
                    break;
                  case 3:
                    renderer = RenderFunction_Subtractive;
                    break;
                  case 5:
                    renderer = RenderFunction_Screen;
                    break;
                  case 8:
                    renderer = RenderFunction_Merge;
                    break;
                  }
                  if (pCurrent->Params.Beam) {
                    w /= 2;
                    FPoint s, e;
                    FPoint fv, lv, rv;
                    float xc, yc;
                    xc = pCurrent->Graphic.XCenter - (pCurrent->Graphic.Rectangle.Width / 2.0f);
                    yc = pCurrent->Graphic.YCenter - (pCurrent->Graphic.Rectangle.Height);
                    s = FPoint((pCurrent->Position.AX * Camera->ParallaxX) - Camera->ViewportX + xc, 
                      (pCurrent->Position.AY * Camera->ParallaxY) - Camera->ViewportY + yc);
                    e = FPoint((pCurrent->Position.X * Camera->ParallaxX) - Camera->ViewportX + xc, 
                      (pCurrent->Position.Y * Camera->ParallaxY) - Camera->ViewportY + yc);
                    fv = e;
                    fv -= s;
                    float repeats = fv.length() / pCurrent->Graphic.Rectangle.Height;
                    h = repeats * pCurrent->Graphic.Rectangle.Height;
                    fv.normalize();
                    lv = fv.rotate90l();
                    lv *= w;
                    rv = fv.rotate90r();
                    rv *= w;
                    FPoint pt;
                    poly.Empty();
                    pt = e; pt += lv;
                    poly.Append(TexturedVertex(pt, 
                      pCurrent->Graphic.Rectangle.Left, pCurrent->Graphic.Rectangle.Top + h));
                    pt = e; pt += rv;
                    poly.Append(TexturedVertex(pt, 
                      pCurrent->Graphic.Rectangle.right_exclusive(), pCurrent->Graphic.Rectangle.Top + h));
                    pt = s; pt += rv;
                    poly.Append(TexturedVertex(pt, 
                      pCurrent->Graphic.Rectangle.right_exclusive(), pCurrent->Graphic.Rectangle.Top));
                    pt = s; pt += lv;
                    poly.Append(TexturedVertex(pt, 
                      pCurrent->Graphic.Rectangle.Left, pCurrent->Graphic.Rectangle.Top));
                    ScalerFunction* sampler = DefaultSampleFunction;
                    if ((sampler == SampleRow_Bilinear) || (sampler == SampleRow_Bilinear_Rolloff))
                      sampler = SampleRow_Bilinear_Wrap;
                    if ((sampler == SampleRow_Linear) || (sampler == SampleRow_Linear_Rolloff))
                      sampler = SampleRow_Linear_Wrap;
                    FilterSimple_ConvexPolygon_Textured(pTarget, currentImage, &poly, sampler, renderer, pCurrent->Params.Color.V);
                  } else {
                    x = (pCurrent->Position.X * Camera->ParallaxX) - Camera->ViewportX - (pCurrent->Graphic.XCenter - (w/2));
                    y = (pCurrent->Position.Y * Camera->ParallaxY) - Camera->ViewportY + (h) - pCurrent->Graphic.YCenter;
                    r *= Radian;
                    y -= (h * s / 2);
                    s /= 2;
                    w *= abs(s); h *= s;
                    poly.Empty();
                    Rotate4Points(w, h, r, px, py);
                    poly.Append(TexturedVertex(px[0] + x, py[0] + y, pCurrent->Graphic.Rectangle.Left, pCurrent->Graphic.Rectangle.Top));
                    poly.Append(TexturedVertex(px[1] + x, py[1] + y, pCurrent->Graphic.Rectangle.right_exclusive(), pCurrent->Graphic.Rectangle.Top));
                    poly.Append(TexturedVertex(px[2] + x, py[2] + y, pCurrent->Graphic.Rectangle.right_exclusive(), pCurrent->Graphic.Rectangle.bottom_exclusive()));
                    poly.Append(TexturedVertex(px[3] + x, py[3] + y, pCurrent->Graphic.Rectangle.Left, pCurrent->Graphic.Rectangle.bottom_exclusive()));
                    FilterSimple_ConvexPolygon_Textured(pTarget, currentImage, &poly, DefaultSampleFunction, renderer, pCurrent->Params.Color.V);
                  }
                } else {
                  if (pCurrent->Params.Scale < 0) {
                    w = w + 1;
                  }
                  w *= abs(pCurrent->Params.Scale) / 2;
                  rctDest.Left = ceil(x - (w));
                  rctDest.Top = ceil(y - (h * pCurrent->Params.Scale));
                  rctDest.Width = pCurrent->Graphic.Rectangle.Width * abs(pCurrent->Params.Scale);
                  rctDest.Height = pCurrent->Graphic.Rectangle.Height * pCurrent->Params.Scale;
                  rctDest.normalize();
                  if (pCurrent->Params.Scale < 0) { 
                    rctSource = pCurrent->Graphic.Rectangle;
                    rctSource.Top += rctSource.Height;
                    rctSource.Height = -rctSource.Height;
                  } else {
                    rctSource = pCurrent->Graphic.Rectangle;
                  }
                  rctCopy = rctDest;
                  if (ClipRectangle_Rect(&rctCopy, &(Camera->Rectangle))) {
                    switch(pCurrent->Params.BlitMode) {
                    default:
                    case 0:
                      BlitResample_SourceAlpha_Opacity(pTarget, currentImage, &rctDest, &rctSource, DefaultSampleFunction, pCurrent->Params.Alpha * Camera->Alpha);
                      break;
                    case 1:
                      BlitResample_SourceAlpha_Opacity(pTarget, currentImage, &rctDest, &rctSource, DefaultSampleFunction, pCurrent->Params.Alpha * Camera->Alpha);
                      break;
                    case 2:
                      BlitResample_Additive_Opacity(pTarget, currentImage, &rctDest, &rctSource, DefaultSampleFunction, pCurrent->Params.Alpha * Camera->Alpha);
                      break;
                    case 3:
                      BlitResample_Subtractive_Opacity(pTarget, currentImage, &rctDest, &rctSource, DefaultSampleFunction, pCurrent->Params.Alpha * Camera->Alpha);
                      break;
                    }
                    if (Options->DrawFrameRectangles) {
                      Rectangle rctOutline = rctDest;
                      FilterSimple_Box(pTarget, &rctOutline, Options->FrameRectangleColor);
                    }
                    if (Options->DrawSortingLines) {
                      FilterSimple_Line_AA(pTarget, rctDest.Left, rctDest.bottom_exclusive() + pCurrent->ZLeft, rctDest.right_exclusive(), rctDest.bottom_exclusive() + pCurrent->ZRight, Options->SortingLineColor);
                    }
                  }
                }
              }
            }
          }
        }
        if ((pCurrent->pAttachedGraphic != Null) && (Options->DrawAttachedGraphics)) {
          if (pCurrent->pAttachedGraphic->pFrames) {
            pImage = pCurrent->pAttachedGraphic->pFrames[ClipValue(pCurrent->pAttachedGraphic->Frame,0,pCurrent->pAttachedGraphic->FrameCount - 1)];
            if (pImage) {
              rctDest.Left = ceil(x - (pCurrent->pAttachedGraphic->XCenter - (pImage->Width/2)));
              rctDest.Top = ceil(y - (h + (float)pImage->Height) - pCurrent->pAttachedGraphic->YCenter);
              rctDest.Width = pImage->Width;
              rctDest.Height = pImage->Height;
              ModedBlit((SFX_BlitModes)(pCurrent->pAttachedGraphic->BlitMode), pTarget, pImage, &rctDest, 0, 0, pCurrent->pAttachedGraphic->Alpha);
            }
          }
        }
        if ((Options->DrawBlocking) && (pCurrent->Stats.Solid)) {
          SimplePolygon* poly = pCurrent->getPolygon();
          poly->Translate(-Camera->ViewportX, -Camera->ViewportY);
          FilterSimple_ConvexPolygon_Outline(pTarget, poly, Options->BlockingColor);
          delete poly;
        }
        if ((Options->DrawOrientationLines) || (Options->DrawVelocityLines)) {
          FPoint start = FPoint(pCurrent->Position.X - Camera->ViewportX, pCurrent->Position.Y - Camera->ViewportY);
          FPoint vel = FPoint(pCurrent->Velocity.X + (sin(pCurrent->Velocity.B * fRadian) * pCurrent->Velocity.V) + pCurrent->Velocity.XF,
                              pCurrent->Velocity.Y + (-cos(pCurrent->Velocity.B * fRadian) * pCurrent->Velocity.V) + pCurrent->Velocity.YF);
          FPoint ori = FPoint((sin(pCurrent->Velocity.B * fRadian) * 10.0f),
                              (-cos(pCurrent->Velocity.B * fRadian) * 10.0f));
          if (Options->DrawOrientationLines)
            FilterSimple_Line_AA(pTarget, start.X, start.Y, start.X + ori.X, start.Y + ori.Y, Options->OrientationLineColor);
          if (Options->DrawVelocityLines)
            FilterSimple_Line_AA(pTarget, start.X, start.Y, start.X + vel.X, start.Y + vel.Y, Options->VelocityLineColor);
        } 
      }
nextsprite:        
      pCurrent = pCurrent->pSortedNext;
      if (pCurrent == Start) break;
    }
    poly.Deallocate();
    return iCount;
}

template <class T> inline T max(T v1, T v2) {
    if (v1 > v2) {
        return v1;
    } else {
        return v2;
    }
}

Export int RenderWindow(Image *Dest, Rectangle *Area, WindowSkinParam * wp, int SectionFlags) {
int xs = 0, ys = 0;
int xm[2] = {0,0}, ym[2] = {0,0};
Rectangle dest, source, clipper, *clip;
Rectangle old_clip;
    if (!Dest) return Failure;
    if (!wp) return Failure;
    if (!wp->pImages) return Failure;
    if (!Area) return Failure;
    if (SectionFlags <= 0) {
      SectionFlags = sfAll;
    }
    int result;
    if (result = Override::EnumOverrides(Override::RenderWindow, 4, Dest, Area, wp, SectionFlags)) {
      return result;
    }
    enableClipping = true;
    old_clip = Dest->ClipRectangle;
    dest = *Area;
    clip = &(Dest->ClipRectangle);
    clipper = *clip;
    clipper.Left = ClipValue(Area->Left - wp->EdgeOffsets[0], clip->Left, clip->right());
    clipper.setRight(ClipValue(Area->right() + wp->EdgeOffsets[2], clip->Left, clip->right()));
    clipper.Top = ClipValue(Area->Top - wp->EdgeOffsets[1], clip->Top, clip->bottom());
    clipper.setBottom(ClipValue(Area->bottom() + wp->EdgeOffsets[3], clip->Top, clip->bottom()));
    Dest->ClipRectangle = clipper;
    xm[0] = _Max(_Max(wp->pImages[wsTopLeft]->Width, wp->pImages[wsLeft]->Width),wp->pImages[wsBottomLeft]->Width);
    xm[1] = _Max(_Max(wp->pImages[wsTopRight]->Width, wp->pImages[wsRight]->Width),wp->pImages[wsBottomRight]->Width);
    ym[0] = _Max(_Max(wp->pImages[wsTopLeft]->Height, wp->pImages[wsTop]->Height),wp->pImages[wsTopRight]->Height);
    ym[1] = _Max(_Max(wp->pImages[wsBottomLeft]->Height, wp->pImages[wsBottom]->Height),wp->pImages[wsBottomRight]->Height);
    xs = wp->pImages[wsMiddle]->Width;
    ys = wp->pImages[wsMiddle]->Height;
    if (SectionFlags & sfMiddle) {
      source = wp->pImages[wsMiddle]->getRectangle();
      dest.setValues(Area->Left - wp->EdgeOffsets[0], Area->Top - wp->EdgeOffsets[1], 
          Area->Width + wp->EdgeOffsets[0] + wp->EdgeOffsets[2], Area->Height + wp->EdgeOffsets[1] + wp->EdgeOffsets[3]);
      switch (wp->BackgroundMode) {
      default:
      case 0:
          if ((xs <= 1) && (ys <= 1)) {
              // alpha fill
              FilterSimple_Fill_SourceAlpha_Opacity(Dest, &dest, wp->pImages[wsMiddle]->getPixel(0,0), wp->Alpha);
          } else {
              // tiled blit
              ModedTiledBlit((SFX_BlitModes)wp->RenderMode, Dest, wp->pImages[wsMiddle], &dest, wp->TintColors[wsMiddle], wp->Alpha);
          }
          break;
      case 1:
          if ((xs <= 1) && (ys <= 1)) {
              // alpha fill
              FilterSimple_Fill_SourceAlpha_Opacity(Dest, &dest, wp->pImages[wsMiddle]->getPixel(0,0), wp->Alpha);
          } else {
              // scaled blit
              ModedResampleBlit((SFX_BlitModes)wp->RenderMode, Dest, wp->pImages[wsMiddle], &dest, &source, wp->TintColors[wsMiddle], wp->Alpha);
          }
          break;
      case 2:
          // gradient
          FilterSimple_Gradient_4Point_SourceAlpha(Dest, &dest, ScaleAlpha(wp->CornerColors[0], wp->Alpha), ScaleAlpha(wp->CornerColors[1], wp->Alpha), ScaleAlpha(wp->CornerColors[2], wp->Alpha), ScaleAlpha(wp->CornerColors[3], wp->Alpha));
          break;
      case 3:
          if ((xs <= 1) && (ys <= 1)) {
              // alpha fill
              FilterSimple_Fill_SourceAlpha_Opacity(Dest, &dest, wp->pImages[wsMiddle]->getPixel(0,0), wp->Alpha);
          } else {
              // tiled blit
              ModedTiledBlit((SFX_BlitModes)wp->RenderMode, Dest, wp->pImages[wsMiddle], &dest, wp->TintColors[wsMiddle], wp->Alpha);
          }
          FilterSimple_Gradient_4Point_SourceAlpha(Dest, &dest, ScaleAlpha(wp->CornerColors[0], wp->Alpha), ScaleAlpha(wp->CornerColors[1], wp->Alpha), ScaleAlpha(wp->CornerColors[2], wp->Alpha), ScaleAlpha(wp->CornerColors[3], wp->Alpha));
          break;
      case 4:
          if ((xs <= 1) && (ys <= 1)) {
              // alpha fill
              FilterSimple_Fill_SourceAlpha_Opacity(Dest, &dest, wp->pImages[wsMiddle]->getPixel(0,0), wp->Alpha);
          } else {
              // scaled blit
              ModedResampleBlit((SFX_BlitModes)wp->RenderMode, Dest, wp->pImages[wsMiddle], &dest, &source, wp->TintColors[wsMiddle], wp->Alpha);
          }
          FilterSimple_Gradient_4Point(Dest, &dest, ScaleAlpha(wp->CornerColors[0], wp->Alpha), ScaleAlpha(wp->CornerColors[1], wp->Alpha), ScaleAlpha(wp->CornerColors[2], wp->Alpha), ScaleAlpha(wp->CornerColors[3], wp->Alpha));
          break;
      }
    }

    Dest->ClipRectangle = old_clip;

    if (SectionFlags & sfTop) {
      dest.setValues(Area->Left, Area->Top - wp->pImages[wsTop]->Height, Area->Width, wp->pImages[wsTop]->Height);
      ModedTiledBlit((SFX_BlitModes)wp->RenderMode, Dest, wp->pImages[wsTop], &dest, wp->TintColors[wsTop], wp->Alpha);
    }

    if (SectionFlags & sfBottom) {
      dest.setValues(Area->Left, Area->bottom(), Area->Width, wp->pImages[wsBottom]->Height);
      ModedTiledBlit((SFX_BlitModes)wp->RenderMode, Dest, wp->pImages[wsBottom], &dest, wp->TintColors[wsBottom], wp->Alpha);
    }

    if (SectionFlags & sfLeft) {
      dest.setValues(Area->Left - wp->pImages[wsLeft]->Width, Area->Top, wp->pImages[wsLeft]->Width, Area->Height);
      ModedTiledBlit((SFX_BlitModes)wp->RenderMode, Dest, wp->pImages[wsLeft], &dest, wp->TintColors[wsLeft], wp->Alpha);
    }

    if (SectionFlags & sfRight) {
      dest.setValues(Area->right(), Area->Top, wp->pImages[wsRight]->Width, Area->Height);
      ModedTiledBlit((SFX_BlitModes)wp->RenderMode, Dest, wp->pImages[wsRight], &dest, wp->TintColors[wsRight], wp->Alpha);
    }

    Dest->ClipRectangle = old_clip;
      
    if (SectionFlags & sfBottomRight) {
      dest.setValues(Area->right(), Area->bottom(), wp->pImages[wsBottomRight]->Width, wp->pImages[wsBottomRight]->Height);
      ModedBlit((SFX_BlitModes)wp->RenderMode, Dest, wp->pImages[wsBottomRight], &dest, 0, 0, wp->TintColors[wsBottomRight], wp->Alpha);
    }

    if (SectionFlags & sfBottomLeft) {
      dest.setValues(Area->Left - wp->pImages[wsBottomLeft]->Width, Area->bottom(), wp->pImages[wsBottomLeft]->Width, wp->pImages[wsBottomLeft]->Height);
      ModedBlit((SFX_BlitModes)wp->RenderMode, Dest, wp->pImages[wsBottomLeft], &dest, 0, 0, wp->TintColors[wsBottomLeft], wp->Alpha);
    }

    if (SectionFlags & sfTopRight) {
      dest.setValues(Area->right(), Area->Top - wp->pImages[wsTopRight]->Height, wp->pImages[wsTopRight]->Width, wp->pImages[wsTopRight]->Height);
      ModedBlit((SFX_BlitModes)wp->RenderMode, Dest, wp->pImages[wsTopRight], &dest, 0, 0, wp->TintColors[wsTopRight], wp->Alpha);
    }

    if (SectionFlags & sfTopLeft) {
      dest.setValues(Area->Left - wp->pImages[wsTopLeft]->Width, Area->Top - wp->pImages[wsTopLeft]->Height, wp->pImages[wsTopLeft]->Width, wp->pImages[wsTopLeft]->Height);
      ModedBlit((SFX_BlitModes)wp->RenderMode, Dest, wp->pImages[wsTopLeft], &dest, 0, 0, wp->TintColors[wsTopLeft], wp->Alpha);
    }

    Dest->ClipRectangle = old_clip;

    return true;
}

Export int RenderText(wchar_t *Text, Image *Dest, Rectangle *Rect, FontParam *Font, TextParam *Options) {
  basic_string<wchar_t> buffer;
  basic_string<wchar_t> color_buffer;
  basic_string<wchar_t> name_buffer;
  wchar_t *current;
  FontParam *currentFont, *nextFont;
  CharacterParam *_current = Null;
  Rectangle dest, clip, shadow, old_clip;
  Pixel shadowColor, textColor, nextColor;
  // configuration/data flags
  bool done = false, invisible = false, draw_shadow = false, broke = false;
  bool draw_caret = false, draw_selection = false, locate_char = false;
  // states
  bool _break = false, _render = false, _move_down = false;
  int line_count = 0;
  int current_X = 0, maximum_X = 0;
  int index = 1, buffer_index = 1;
  int total_y = 0;
  int charsDrawn = 0;
  int currentChar = 0;
  if (!Text) return Failure;
  if (!Rect) return Trivial_Success;
  if (!Font) return Failure;
  if (!(Font->MapPointer)) return Failure;
  if (Font->MapCount < 1) return Failure;

  clip = *Rect;

  if (!Dest) {
    invisible = true;
  } else {
    old_clip = Dest->ClipRectangle;
//    ClipRectangle_ImageClipRect(&clip, Dest);
    Dest->clipRectangle(&clip);
    if (clip.empty()) return Trivial_Success;
    Dest->setClipRectangle(&clip);
  }

  if (!invisible) {
    draw_caret = (Options->Caret_Position > 0);
    draw_selection = ((Options->Selection_End > Options->Selection_Start) && (Options->Selection_Start > 0));
  }

  if (Options->CharFromPoint == 1) {
    Options->CharFromPoint = 0;
    locate_char = true;
  }

  currentFont = Font;
  nextFont = Font;

  enableClipping = true;
  nextColor = textColor = Font->FillColor;
  shadowColor = Font->ShadowColor;
  draw_shadow = (shadowColor[::Alpha] > 0);
  
  int X = 0, buffer_X = Rect->Left + Options->Scroll_X;
  int buffer_Y = Rect->Top + Options->Scroll_Y;
  int buffer_width = 0, last_width = 0, last_X = 0;
  int row_height = Font->BaseHeight;
  name_buffer.clear();
  color_buffer.clear();
  buffer.clear();

  current = Text;

  while (!done) {
    _break = false;
    _render = false;
    _move_down = false;

    if ((buffer_Y) >= Rect->bottom()) invisible = true;

    _current = (*current <= currentFont->MapCount) ? currentFont->MapPointer[*current] : currentFont->MapPointer[32];

    if (*((unsigned char *)current) == 0) {
      _render = true;
      done = true;
      broke = false;
    }

    if (!done) {
      switch (*current) {
        case 0:
          // null (who cares)
          break;
        case 10:
          // line feed (ignore)
          break;
        case 8:
          // backspace
          if (buffer_width > 0) {
            buffer_width = last_width;
          } else {
            buffer_X = last_X;
          }
          break;
        case 13:
          // carriage return
          _break = true;
          break;
        case '#':
          if (((*(current)) == '#') && ((*(current+1)) == '[') && (Options->EnableColorCodes)) {
            // parsity parse parse parse
            color_buffer.clear();
            current++;
            while (1) {
              if ((*current) == 0) {
                break;
              } else if ((*current) == '[') {
              } else if ((*current) == ']') {
                break;
              } else {
                color_buffer += *current;
              }
              current++;
            }
            _render = true;
            if ((*current) == 0) {
              _render = true;
              done = true;
              broke = false;
              break;
            }
            if (color_buffer.size() < 2) {
              nextColor = currentFont->FillColor;
            } else {
              nextColor = Pixel(color_buffer);
            }
            break;
          } else {
            // pass this on
          }
        case '~':
          if (((*(current)) == '~') && ((*(current+1)) == '[') && (Options->EnableColorCodes)) {
            // parsity parse parse parse
            name_buffer.clear();
            current++;
            while (1) {
              if ((*current) == 0) {
                break;
              } else if ((*current) == '[') {
              } else if ((*current) == ']') {
                break;
              } else {
                name_buffer += *current;
              }
              current++;
            }
            _render = true;
            if ((*current) == 0) {
              _render = true;
              done = true;
              broke = false;
              break;
            }
            if (name_buffer.size() < 1) {
              nextFont = Font;
            } else {
              nextFont = Font;
              if (Font->SubFontCount) {
                for (int f = 0; f < Font->SubFontCount; f++) {
                  if (_wcsicmp(name_buffer.c_str(), Font->SubFonts[f].Name) == 0) {
                    nextFont = Font->SubFonts[f].Font;
                    break;
                  }
                }
              } 
              if (nextFont == 0) nextFont = Font;
              row_height = _Max(_Max(row_height, nextFont->BaseHeight), Font->BaseHeight);
            }
            break;
          } else {
            // pass this on
          }
        case ' ': case '.':
        case ',': case ';':
        case '-':
        case '+': case '*':
        case '/': case '=':
        case ':': case '(':
        case ')': case '<':
        case '[': case ']':
        case '>': case '!':
        case  9 : case '?':
        case '@': case '^':
        case '_': case '`':
          broke = false;
          buffer += (*current);
		      if (*current == 9) {
			      // tab
			      bool stopFound = false;
			      if ((Options->TabStops != Null) && (Options->TabStopCount > 0)) {
			        for (int t = 0; t < Options->TabStopCount; t++) {
			          if (Options->TabStops[t] > (buffer_width + buffer_X - Rect->Left + Options->Scroll_X)) {
			            buffer_width = Options->TabStops[t] - (buffer_X - Rect->Left + Options->Scroll_X);
			            stopFound = true;
			            break;
			          }
			        }
			      }
			      if (!stopFound) {
  		        buffer_width += _current->XIncrement;
			      }
		      } else {
	          if (_current) {
  		        buffer_width += _current->XIncrement;
			      }
		      }
          if (((buffer_width + current_X + Options->Scroll_X) > Rect->Width)) {
            if (currentFont->WrapMode == 1) { 
              done = true;
              _break = true;
              _render = true;
            } else {
              if ((buffer_width) >= Rect->Width) {
                _break = true;
                _render = true;
              } else {
                _move_down = true;
              }
            }
          }
          _render = true;
          break;
        default:
          buffer += (*current);
          if (_current) {
            buffer_width += _current->XIncrement;
          }
          if (((buffer_width + current_X + Options->Scroll_X) > Rect->Width)) {
            if ((buffer_width) >= Rect->Width) {
              _render = true;
              _break = true;
            } else {
              _move_down = true;
            }
          }
          break;
      }
    }

    _render = _render || _break;

    if ((_move_down) && (currentFont->WrapMode == 0)) {
      if (current_X > maximum_X) maximum_X = current_X;
      current_X = 0;
      buffer_X = Rect->Left + Options->Scroll_X;
      buffer_Y += row_height;
      if (buffer_width >= Rect->Width) _render = true;
      line_count++;
      total_y += row_height;
      broke = true;
      row_height = _Max(Font->BaseHeight, currentFont->BaseHeight);
    }

    if (_render) {
      X = buffer_X;
      for (DoubleWord i = 0; i < buffer.size(); i++) {
        _current = ((buffer[i] <= currentFont->MapCount) && (buffer[i] >= 0)) ? currentFont->MapPointer[buffer[i]] : currentFont->MapPointer[32];
		    if ((currentChar > Options->MaxChars) && (Options->MaxChars > 0)) invisible = true;

        if ((_current) && (!invisible)) {
          if (draw_caret) {
            if (buffer_index == Options->Caret_Position) {
              dest.setValues(X, buffer_Y, 1, row_height);
              FilterSimple_Fill_SourceAlpha(Dest, &dest, ScaleAlpha(Options->Caret_Color, Options->Opacity));
            }
          }

          if (draw_selection) {
            if ((buffer_index >= Options->Selection_Start) && (buffer_index < Options->Selection_End)) {
              dest.setValues(X, buffer_Y, _current->XIncrement, row_height);
              FilterSimple_Fill_SourceAlpha(Dest, &dest, ScaleAlpha(Options->Selection_Color, Options->Opacity));
            }
          }

          if (_current->pImage) {
            dest.Left = X + _current->XOffset;
            if (currentFont->BaseMode == 1) {
              dest.Top = buffer_Y + _current->YOffset;
            } else {
              dest.Top = buffer_Y + _current->YOffset + (currentFont->BaseHeight - _current->pImage->Height);
            }
            dest.Width = _current->pImage->Width;
            dest.Height = _current->pImage->Height;

            if (draw_shadow) {
              shadow = dest;
              shadow.Left += 1;
              shadow.Top += 1;
              
              if (currentFont->EffectMode == 1) {
                BlitSimple_Font_Merge_RGB_Opacity(Dest, _current->pImage, &shadow, 0, 0, shadowColor, AlphaLookup(Font->Alpha, ClipByte(Options->Opacity)));
              } else {
                BlitSimple_Font_SourceAlpha_RGB_Opacity(Dest, _current->pImage, &shadow, 0, 0, shadowColor, AlphaLookup(Font->Alpha, ClipByte(Options->Opacity)));
              }
            }

			      charsDrawn++;
            if (currentFont->EffectMode == 1) {
              if (textColor.V == (DoubleWord)0xFFFFFFFF) {
                BlitSimple_Merge_Opacity(Dest, _current->pImage, &dest, 0, 0, AlphaLookup(Font->Alpha, ClipByte(Options->Opacity)));
              } else {
                BlitSimple_Font_Merge_RGB_Opacity(Dest, _current->pImage, &dest, 0, 0, textColor, AlphaLookup(Font->Alpha, ClipByte(Options->Opacity)));
              }
            } else {
              if (textColor.V == (DoubleWord)0xFFFFFFFF) {
                BlitSimple_Automatic_SourceAlpha_Opacity(Dest, _current->pImage, &dest, 0, 0, AlphaLookup(Font->Alpha, ClipByte(Options->Opacity)));
              } else {
                BlitSimple_Font_SourceAlpha_RGB_Opacity(Dest, _current->pImage, &dest, 0, 0, textColor, AlphaLookup(Font->Alpha, ClipByte(Options->Opacity)));
              }
            }
          }

        }

        if (_current) {
          if (locate_char) {
            if ((Options->CharFromPoint_X < (X + _current->XIncrement))) {
//            if ((Options->CharFromPoint_X >= X) && (Options->CharFromPoint_X < (X + _current->XIncrement))) {
              if ((Options->CharFromPoint_Y >= buffer_Y) && (Options->CharFromPoint_Y < (buffer_Y + row_height))) {
                Options->CharFromPoint = buffer_index;
                if (Dest) {
                  Dest->ClipRectangle = old_clip;
                }
                return Success;
              }
            }
          }
          X += _current->XIncrement;
          current_X += _current->XIncrement;
        }
        currentChar++;
        buffer_index++;
      }
      if (current_X > maximum_X) maximum_X = current_X;
      if (done && draw_caret) {
        if (buffer_index == Options->Caret_Position) {
          dest.setValues(X, buffer_Y, 1, row_height);
          FilterSimple_Fill_SourceAlpha(Dest, &dest, ScaleAlpha(Options->Caret_Color, ClipByte(Options->Opacity)));
        }
      }
      if (done && locate_char) {
        if ((Options->CharFromPoint_X > X) || (Options->CharFromPoint_Y > buffer_Y)) {
          Options->CharFromPoint = buffer_index;
          if (Dest) {
            Dest->ClipRectangle = old_clip;
          }
          return Success;
        }
      }
      buffer_X = buffer_X + buffer_width;
      buffer.clear();
      buffer_width = 0;
      buffer_index = index + 1;
      if (done) {
        line_count++;
        row_height = _Max(Font->BaseHeight, currentFont->BaseHeight);
        total_y += row_height;
      }
    }

    if ((_break) && (currentFont->WrapMode == 0)) {
      broke = true;
      if (current_X > maximum_X) maximum_X = current_X;
      current_X = 0;
      buffer_X = Rect->Left + Options->Scroll_X;
      buffer_Y += row_height;
      buffer_width = 0;
      line_count++;
      total_y += row_height;
      row_height = _Max(Font->BaseHeight, currentFont->BaseHeight);
    }

    current++;
    index++;

    last_width = buffer_width;
    last_X = buffer_X;

    if (nextFont != currentFont) {
      currentFont = nextFont;
    }

    if (nextColor != textColor) {
      textColor = nextColor;
      shadowColor = currentFont->ShadowColor;
      draw_shadow = (shadowColor[::Alpha] > 0);
    }

  }

  if (current_X > maximum_X) maximum_X = current_X;

  if (Dest) {
    Dest->ClipRectangle = old_clip;
  }

  Options->Width = maximum_X;
  Options->Height = total_y;
  Options->Lines = line_count;
  return Success;
}

Export int FindSpriteOnscreen(SpriteParam *first, FRect *area, SpriteParam *exclude, bool mustbesolid, int requiredtype, int excludedtype) {
SpriteParam * sCurrent = first;
float x, y, w, h;
    if (!first) return 0;
    if (!area) return 0;
FRect rSource = *area, rDest;
    while (sCurrent) {
        if (sCurrent != exclude) {
            if ((sCurrent->Type != excludedtype) && (((requiredtype >= 0) && (sCurrent->Type == requiredtype)) || (requiredtype < 0))) {
                if ((sCurrent->Stats.Solid) || (!mustbesolid)) {
					          w = sCurrent->Graphic.Rectangle.Width / 2;
					          h = sCurrent->Graphic.Rectangle.Height;
					          x = sCurrent->Position.X - (sCurrent->Graphic.XCenter - w);
					          y = sCurrent->Position.Y + (h) - sCurrent->Graphic.YCenter;
                    rDest.X1 = x - w;
                    rDest.X2 = x + w;
                    rDest.Y1 = (sCurrent->Position.Y) - (h);
                    rDest.Y2 = (sCurrent->Position.Y);
                    if (rSource.X1 > rDest.X2) goto nevar;
                    if (rSource.Y1 > rDest.Y2) goto nevar;
                    if (rSource.X2 < rDest.X1) goto nevar;
                    if (rSource.Y2 < rDest.Y1) goto nevar;
                    return sCurrent->Index;
                }
            }
        }
nevar:
        sCurrent = sCurrent->pNext;
    }
    return 0;
}

Export int FindSprite(SpriteParam *first, FRect *area, SpriteParam *exclude, bool mustbesolid, int requiredtype, int excludedtype) {
SpriteParam * sCurrent = first;
    if (!first) return 0;
    if (!area) return 0;
FRect rSource = *area, rDest;
    while (sCurrent) {
        if (sCurrent != exclude) {
            if ((sCurrent->Type != excludedtype) && (((requiredtype >= 0) && (sCurrent->Type == requiredtype)) || (requiredtype < 0))) {
                if ((sCurrent->Stats.Solid) || (!mustbesolid)) {
                    rDest = sCurrent->getRect();
                    if (rSource.X1 > rDest.X2) goto nevar;
                    if (rSource.Y1 > rDest.Y2) goto nevar;
                    if (rSource.X2 < rDest.X1) goto nevar;
                    if (rSource.Y2 < rDest.Y1) goto nevar;
                    return sCurrent->Index;
                }
            }
        }
nevar:
        sCurrent = sCurrent->pNext;
    }
    return 0;
}

Export int GetClosestSprite(SpriteParam *first, SpriteParam *target, bool mustbesolid, int requiredtype, int excludedtype, float *outdistance) {
int closest = 0;
float closestdist = 999999999.0;
float xdist = 0, ydist = 0, dist = 0;
SpriteParam * sCurrent = first;
    if (!first) return 0;
    if (!target) return 0;
    while (sCurrent) {
        if (sCurrent != target) {
            if ((sCurrent->Type != excludedtype) && (((requiredtype >= 0) && (sCurrent->Type == requiredtype)) || (requiredtype < 0))) {
                if ((sCurrent->Stats.Solid) || (!mustbesolid)) {
                    xdist = abs(sCurrent->Position.X - target->Position.X);
                    ydist = abs(sCurrent->Position.Y - target->Position.Y);
                    dist = sqrt(xdist * ydist);
                    if (dist < closestdist) {
                        closestdist = dist;
                        closest = sCurrent->Index;
                    }
                }
            }
        }
        sCurrent = sCurrent->pNext;
    }
    *outdistance = closestdist;
    return closest;
}

Export int GetFarthestSprite(SpriteParam *first, SpriteParam *target, bool mustbesolid, int requiredtype, int excludedtype, float *outdistance) {
int farthest = 0;
float farthestdist = 0.0;
float xdist = 0, ydist = 0, dist = 0;
SpriteParam * sCurrent = first;
    if (!first) return 0;
    if (!target) return 0;
    while (sCurrent) {
        if (sCurrent != target) {
            if ((sCurrent->Type != excludedtype) && (((requiredtype >= 0) && (sCurrent->Type == requiredtype)) || (requiredtype < 0))) {
                if ((sCurrent->Stats.Solid) || (!mustbesolid)) {
                    xdist = abs(sCurrent->Position.X - target->Position.X);
                    ydist = abs(sCurrent->Position.Y - target->Position.Y);
                    dist = sqrt(xdist * ydist);
                    if (dist > farthestdist) {
                        farthestdist = dist;
                        farthest = sCurrent->Index;
                    }
                }
            }
        }
        sCurrent = sCurrent->pNext;
    }
    *outdistance = farthestdist;
    return farthest;
}

//
// cohen sutherland
//

inline int PointRegionCode(FPoint *pt, FRect *rgn) {
int value = 0;
    if (pt->X < rgn->X1) {
        value |= csLeft;
    }
    else if (pt->X > rgn->X2) {
        value |= csRight;
    }
    if (pt->Y < rgn->Y1) {
        value |= csTop;
    }
    else if (pt->Y > rgn->Y2) {
        value |= csBottom;
    }
    return value;
}

Export int CheckLineCollide(FRect *rct, FLine *lines, int linecount) {
int sCode = 0, eCode = 0, nCode = 0;
FLine *line = lines;
int iCount;
FPoint pt;
FLine ln;
    if (!rct) return false;
    if (!lines) return false;
    for (iCount = 1; iCount <= linecount; iCount++) {
        ln = *line;
        while(true) {
            sCode = PointRegionCode(&ln.Start, rct);
            eCode = PointRegionCode(&ln.End, rct);
            if ((sCode | eCode) == 0) {
                // trivial accept
                return iCount;
            } else if ((sCode & eCode) != 0) {
                // trivial reject
                break;
            } else {
                if(sCode == 0)
                {
                    pt = ln.Start;
                    ln.Start = ln.End;
                    ln.End = pt;
                    nCode = sCode; 
                    sCode = eCode;
                    eCode = nCode;
                } 
                // nontrivial
                if (sCode & csTop) {
                    // top
                    ln.Start.X +=(ln.End.X - ln.Start.X) * (rct->Y1 - ln.Start.Y) / (ln.End.Y - ln.Start.Y);
                    ln.Start.Y = rct->Y1;
                } else if (sCode & csBottom) {
                    // bottom
                    ln.Start.X +=(ln.End.X - ln.Start.X) * (rct->Y2 - ln.Start.Y) / (ln.End.Y - ln.Start.Y);
                    ln.Start.Y = rct->Y2;
                } else if (sCode & csRight) {
                    // right
                    ln.Start.Y +=(ln.End.Y - ln.Start.Y) * (rct->X2 - ln.Start.X) / (ln.End.X - ln.Start.X);
                    ln.Start.X = rct->X2;
                } else {
                    // left
                    ln.Start.Y +=(ln.End.Y - ln.Start.Y) * (rct->X1 - ln.Start.X) / (ln.End.X - ln.Start.X);
                    ln.Start.X = rct->X1;
                }
            }
        }
        line++;
    }
    return false;
}

Export int CheckLineCollide2(FRect *unused, SimplePolygon *poly, FLine *lines, int linecount) {
FLine *line = lines;
int iCount;
FPoint pt;
FLine ln;
    if (!poly) return false;
    if (!lines) return false;
    for (iCount = 1; iCount <= linecount; iCount++) {
        ln = *line;
        if (Intersects(*poly, ln))
          return iCount;
        line++;
    }
    return false;
}

int CheckLineCollide(FRect *rct, std::vector<FLine> Lines) {
int sCode = 0, eCode = 0, nCode = 0;
DoubleWord iCount;
FPoint pt;
FLine ln;
    if (!rct) return false;
    for (iCount = 0; iCount < Lines.size(); iCount++) {
        ln = Lines[iCount];
        while(true) {
            sCode = PointRegionCode(&ln.Start, rct);
            eCode = PointRegionCode(&ln.End, rct);
            if ((sCode | eCode) == 0) {
                // trivial accept
                return iCount + 1;
            } else if ((sCode & eCode) != 0) {
                // trivial reject
                break;
            } else {
                if(sCode == 0)
                {
                    pt = ln.Start;
                    ln.Start = ln.End;
                    ln.End = pt;
                    nCode = sCode; 
                    sCode = eCode;
                    eCode = nCode;
                } 
                // nontrivial
                if (sCode & csTop) {
                    // top
                    ln.Start.X +=(ln.End.X - ln.Start.X) * (rct->Y1 - ln.Start.Y) / (ln.End.Y - ln.Start.Y);
                    ln.Start.Y = rct->Y1;
                } else if (sCode & csBottom) {
                    // bottom
                    ln.Start.X +=(ln.End.X - ln.Start.X) * (rct->Y2 - ln.Start.Y) / (ln.End.Y - ln.Start.Y);
                    ln.Start.Y = rct->Y2;
                } else if (sCode & csRight) {
                    // right
                    ln.Start.Y +=(ln.End.Y - ln.Start.Y) * (rct->X2 - ln.Start.X) / (ln.End.X - ln.Start.X);
                    ln.Start.X = rct->X2;
                } else {
                    // left
                    ln.Start.Y +=(ln.End.Y - ln.Start.Y) * (rct->X1 - ln.Start.X) / (ln.End.X - ln.Start.X);
                    ln.Start.X = rct->X1;
                }
            }
        }
    }
    return false;
}

int CheckLineCollide2(FRect *unused, SimplePolygon *poly, std::vector<FLine> Lines) {
DoubleWord iCount;
FPoint pt;
FLine ln;
    if (!poly) return false;
    for (iCount = 0; iCount < Lines.size(); iCount++) {
        ln = Lines[iCount];
        if (Intersects(*poly, ln))
          return iCount + 1;
    }
    return false;
}

FRect SpriteParam::getRect() {
float w = this->Obstruction.W / 2;
float h = this->Obstruction.H / 2;
FRect v;
  if (isPolygonal()) {
    SimplePolygon* poly = this->getPolygon();
    poly->GetBounds(v);
    delete poly;
    return v;
  } else {
    switch (this->Obstruction.Type) {
      case sotUpwardRect:
        v.X1 = (this->Position.X) - (w);
        v.X2 = (this->Position.X) + (w);
        v.Y1 = (this->Position.Y) - (this->Obstruction.H);
        v.Y2 = (this->Position.Y);
        break;
      case sotCenteredRect:
        v.X1 = (this->Position.X) - (w);
        v.X2 = (this->Position.X) + (w);
        v.Y1 = (this->Position.Y) - (h);
        v.Y2 = (this->Position.Y) + (h);
        break;
      case sotCenteredSphere:
        float s = w;
        v.X1 = (this->Position.X) - (s);
        v.X2 = (this->Position.X) + (s);
        v.Y1 = (this->Position.Y) - (s);
        v.Y2 = (this->Position.Y) + (s);
        break;
    }
    return v;
  }
}

Polygon<FPoint>* SpriteParam::getPolygon() {
float w = (this->Obstruction.W) / 2.0f;
float h = (this->Obstruction.H) / 2.0f;
FRect r;
Polygon<FPoint> *v = new Polygon<FPoint>();
Polygon<FPoint> *base;
    switch (this->Obstruction.Type) {
      case sotUpwardRect:
        r.X1 = (this->Position.X) - (w);
        r.X2 = (this->Position.X) + (w);
        r.Y1 = (this->Position.Y) - (this->Obstruction.H);
        r.Y2 = (this->Position.Y);
        v->Allocate(4);
        v->Append(r.TopLeft());
        v->Append(r.TopRight());
        v->Append(r.BottomRight());
        v->Append(r.BottomLeft());
        break;
      case sotCenteredSphere:
      case sotCenteredRect:
        r.X1 = (this->Position.X) - (w);
        r.X2 = (this->Position.X) + (w);
        r.Y1 = (this->Position.Y) - (h);
        r.Y2 = (this->Position.Y) + (h);
        v->Allocate(4);
        v->Append(r.TopLeft());
        v->Append(r.TopRight());
        v->Append(r.BottomRight());
        v->Append(r.BottomLeft());
        break;
      case sotCenteredPolygon:
        base = (Polygon<FPoint>*)this->Obstruction.Polygon;
        v->Copy(*base);
        v->Translate(this->Position.X, this->Position.Y);
        break;
      case sotBeam:
        FPoint s, e;
        FPoint fv, lv, rv;
        float w = this->Obstruction.W / 2.0f, h = this->Obstruction.H / 2.0f;
        s = FPoint(this->Position.AX, this->Position.AY);
        e = FPoint(this->Position.X, this->Position.Y);
        fv = e;
        fv -= s;
        fv.normalize();
        lv = fv.rotate90l();
        lv *= w;
        rv = fv.rotate90r();
        rv *= w;
        fv *= h;
        s -= fv;
        e += fv;
        FPoint pt;
        v->Allocate(4);
        pt = s; pt += lv;
        v->Append(pt);
        pt = s; pt += rv;
        v->Append(pt);
        pt = e; pt += rv;
        v->Append(pt);
        pt = e; pt += lv;
        v->Append(pt);
        break;
    }
    return v;
}

bool SpriteParam::isPolygonal() const {
  switch (this->Obstruction.Type) {
    case sotUpwardRect:
    case sotCenteredSphere:
    case sotCenteredRect:
      return false;
    case sotCenteredPolygon:
    case sotBeam:
      return true;
  }
  return false;
}

bool SpriteParam::touches(SpriteParam *other) {
  if (this->isPolygonal() || other->isPolygonal()) {
    Polygon<FPoint> *pOther = other->getPolygon();
    bool result = this->touches(pOther);
    delete pOther;
    return result;
  } else {
    FRect rOther;
    rOther = other->getRect();
    return this->touches(&rOther);
  }
  return false;
}

bool SpriteParam::touches(FRect *other) {
FRect rMe;
  rMe = this->getRect();
  return rMe.intersect(other);
}

bool SpriteParam::touches(SimplePolygon *other) {
  Polygon<FPoint> *pMe = this->getPolygon();
  bool result = Intersects(*pMe, *other);
  delete pMe;
  return result;
}

bool SpriteParam::touches(SpriteParam *other, VelocityVector *other_speed) {
  // NYI
  return this->touches(other);
  return false;
}

inline int SpriteParam::touches(FLine *lines, int line_count) {
FRect rct = this->getRect();
  if (this->isPolygonal()) {
    Polygon<FPoint> *poly = this->getPolygon();
    int result = CheckLineCollide2(&rct, poly, lines, line_count);
    delete poly;
    return result;
  } else {
    return CheckLineCollide(&rct, lines, line_count);
  }
}

inline int SpriteParam::touches(CollisionMatrix *Matrix) {
FRect rct;
  rct = this->getRect();
  if (this->isPolygonal()) {
    Polygon<FPoint> *poly = this->getPolygon();
    bool result = Matrix->collisionCheck(&rct, poly);
    delete poly;
    return result;
  } else {
    return Matrix->collisionCheck(&rct);
  }
}

Export int RenderPlaneOutlines(Image *Image, Lighting::Plane *Planes, Pixel Color, int Count, float XOffset, float YOffset) {
  if (!Image) return Failure;
  if (!Planes) return Failure;
  if (Count < 1) return Failure;
  Lighting::Plane Plane;
  for (int p = 0; p < Count; p++) {
    Plane = Planes[p];
    Plane.Start.X -= XOffset;
    Plane.Start.Y -= YOffset;
    Plane.End.X -= XOffset;
    Plane.End.Y -= YOffset;
    FilterSimple_Line_AA(Image, Plane.Start.X, Plane.Start.Y, Plane.End.X, Plane.Start.Y, Color);
    FilterSimple_Line_AA(Image, Plane.Start.X, Plane.Start.Y, Plane.Start.X, Plane.End.Y, Color);
    FilterSimple_Line_AA(Image, Plane.End.X, Plane.Start.Y, Plane.End.X, Plane.End.Y, Color);
    FilterSimple_Line_AA(Image, Plane.Start.X, Plane.End.Y, Plane.End.X, Plane.End.Y, Color);
    FilterSimple_Line_AA(Image, Plane.Start.X, Plane.Start.Y - Plane.Height, Plane.End.X, Plane.Start.Y - Plane.Height, Color);
    FilterSimple_Line_AA(Image, Plane.Start.X, Plane.Start.Y - Plane.Height, Plane.Start.X, Plane.End.Y - Plane.Height, Color);
    FilterSimple_Line_AA(Image, Plane.End.X, Plane.Start.Y - Plane.Height, Plane.End.X, Plane.End.Y - Plane.Height, Color);
    FilterSimple_Line_AA(Image, Plane.Start.X, Plane.End.Y - Plane.Height, Plane.End.X, Plane.End.Y - Plane.Height, Color);
  }
  return Success;
}

Export int RenderPlaneOutlines_Masked(Image *Image, Lighting::Plane *Planes, Byte *Mask, Pixel Color, int Count, float XOffset, float YOffset) {
  if (!Image) return Failure;
  if (!Planes) return Failure;
  if (Count < 1) return Failure;
  Lighting::Plane Plane;
  Pixel PlaneColor;
  for (int p = 0; p < Count; p++) {
    Plane = Planes[p];
    Plane.Start.X -= XOffset;
    Plane.Start.Y -= YOffset;
    Plane.End.X -= XOffset;
    Plane.End.Y -= YOffset;
    PlaneColor = ScaleAlpha(Color, Mask[p]);
    FilterSimple_Line_AA(Image, Plane.Start.X, Plane.Start.Y, Plane.End.X, Plane.Start.Y, PlaneColor);
    FilterSimple_Line_AA(Image, Plane.Start.X, Plane.Start.Y, Plane.Start.X, Plane.End.Y, PlaneColor);
    FilterSimple_Line_AA(Image, Plane.End.X, Plane.Start.Y, Plane.End.X, Plane.End.Y, PlaneColor);
    FilterSimple_Line_AA(Image, Plane.Start.X, Plane.End.Y, Plane.End.X, Plane.End.Y, PlaneColor);
    FilterSimple_Line_AA(Image, Plane.Start.X, Plane.Start.Y - Plane.Height, Plane.End.X, Plane.Start.Y - Plane.Height, PlaneColor);
    FilterSimple_Line_AA(Image, Plane.Start.X, Plane.Start.Y - Plane.Height, Plane.Start.X, Plane.End.Y - Plane.Height, PlaneColor);
    FilterSimple_Line_AA(Image, Plane.End.X, Plane.Start.Y - Plane.Height, Plane.End.X, Plane.End.Y - Plane.Height, PlaneColor);
    FilterSimple_Line_AA(Image, Plane.Start.X, Plane.End.Y - Plane.Height, Plane.End.X, Plane.End.Y - Plane.Height, PlaneColor);
  }
  return Success;
}

#define aceil(n) ((n > 0) ? ceil(n) : floor(n))
#define afloor(n) ((n > 0) ? floor(n) : ceil(n))

Export int SightCheck(Lighting::Environment *Env, float FromX, float FromY, float ToX, float ToY, SpriteParam *IgnoreSprite1, SpriteParam *IgnoreSprite2) {
  FLine LightRay;
  FPoint pt = FPoint(FromX, FromY);
  FPoint IntersectionPoint;
  SpriteParam *Sprite;
  FLine SpriteLine;
  int mx1 = 0, my1 = 0, mx2 = 0, my2 = 0, mx = 0, my = 0;
  Lighting::Sector *Sector = Null;
  std::vector<Lighting::Obstruction>::iterator Obstruction;
  LightRay.Start.X = FromX;
  LightRay.Start.Y = FromY;
  LightRay.End.X = ToX;
  LightRay.End.Y = ToY;
	mx1 = ClipValue(_Min(LightRay.Start.X, LightRay.End.X) / (float)Env->Matrix->SectorWidth, Env->Matrix->Width - 1);
	my1 = ClipValue(_Min(LightRay.Start.Y, LightRay.End.Y) / (float)Env->Matrix->SectorHeight, Env->Matrix->Height - 1);
	mx2 = ClipValue(ceil(_Max(LightRay.Start.X, LightRay.End.X) / (float)Env->Matrix->SectorWidth), Env->Matrix->Width - 1);
	my2 = ClipValue(ceil(_Max(LightRay.Start.Y, LightRay.End.Y) / (float)Env->Matrix->SectorHeight), Env->Matrix->Height - 1);
	for (my = my1; my <= my2; my++) {
		for (mx = mx1; mx <= mx2; mx++) {
			Sector = Env->Matrix->getSector(mx, my);
      if (LightRay.intersect(Sector->Obstructions, IntersectionPoint))
        return 0;
			//for (Obstruction = Sector->Obstructions.begin(); Obstruction != Sector->Obstructions.end(); ++Obstruction) {
			//	if (LightRay.intersect((*(FLine*)&(*Obstruction)), IntersectionPoint)) {
   //       return 0;
			//	}
			//}
		}
	}

  if (Env->Sprites != Null) {
    Sprite = Env->Sprites;
    while (Sprite) {
      if (Sprite == IgnoreSprite1) {
        Sprite = Sprite->pNext; 
        continue;
      }
      if (Sprite == IgnoreSprite2) {
        Sprite = Sprite->pNext; 
        continue;
      }
      switch (Sprite->Params.SpecialFX) {
      default:
        Sprite = Sprite->pNext;
        continue;
        break;
      case fxCastShadow:
      case fxCastGraphicShadow:
        break;
      }
      float s = (Sprite->Obstruction.W + Sprite->Obstruction.H) / 4;
      float h = Sprite->Obstruction.H / 2;
      FPoint p = FPoint(Sprite->Position.X, Sprite->Position.Y);
      FPoint vl = FLine(p, pt).vector(), vr;
      float d = vl.length();
      vl /= d;
      vl *= s;
      vr = vl.rotate90r();
      vl = vl.rotate90l();
      p.Y -= h;
      SpriteLine.Start = p;
      SpriteLine.End = p;
      SpriteLine.Start += vl;
      SpriteLine.End += vr;
      if (LightRay.intersect(SpriteLine, IntersectionPoint)) {
        return 0;
      }
      Sprite = Sprite->pNext;
    }
  }
  return 1;
}

Export Pixel RaycastPoint(Lighting::Environment *Env, float X, float Y, SpriteParam *Ignore) {
  if (Env->LightCount < 1) {
    return Env->AmbientLight;
  } else {
    return Lighting::Raycast(Env, X, Y, Ignore, false);
  }
}

Pixel Lighting::Raycast(Lighting::Environment *Env, float X, float Y, SpriteParam *IgnoreSprite, bool EnableCulling) {
  Pixel Color = Env->AmbientLight;
  Pixel LightColor;
  FLine LightRay;
  FPoint IntersectionPoint;
  AlphaLevel *aDistance;
  SpriteParam *Sprite;
  FLine SpriteLine;
  int XDistance, YDistance;
  int Distance;
  float DistanceMultiplier = 0;
  float ldist, ldist11;
  bool Obscured, Falloff;
  int mx1 = 0, my1 = 0, mx2 = 0, my2 = 0, mx = 0, my = 0;
  Lighting::Sector *Sector = Null;
  std::vector<Lighting::Obstruction>::const_iterator Obstruction;
  if (Env->LightCount < 1) {
    return Color;
  }
  float fsw = (float)Env->Matrix->SectorWidth;
  float fsh = (float)Env->Matrix->SectorHeight;
  for (int l = 0; l < Env->LightCount; ++l) {
    Obscured = false;
	  if (((!Env->Lights[l].Culled) || (!Env->Lights[l].PlaneCulled) || (!EnableCulling)) && (Env->Lights[l].Visible)) {
      LightColor = Pixel(0, 0, 0, 0);
      ldist = Env->Lights[l].Data.ldist;
      ldist11 = Env->Lights[l].Data.ldist11;
      Falloff = Env->Lights[l].FalloffDistance > 0;
      if (Falloff) {
        DistanceMultiplier = (255.0F / ldist);
      }
      LightRay.Start.X = Env->Lights[l].X;
      LightRay.Start.Y = Env->Lights[l].Y;
      LightRay.End.X = X;
      LightRay.End.Y = Y;
	    mx1 = ClipValue(floor(_Min(LightRay.Start.X, LightRay.End.X) / fsw), Env->Matrix->Width - 1);
  	  my1 = ClipValue(floor(_Min(LightRay.Start.Y, LightRay.End.Y) / fsh), Env->Matrix->Height - 1);
	    mx2 = ClipValue(ceil(_Max(LightRay.Start.X, LightRay.End.X) / fsw), Env->Matrix->Width - 1);
	    my2 = ClipValue(ceil(_Max(LightRay.Start.Y, LightRay.End.Y) / fsh), Env->Matrix->Height - 1);
      if (Env->Lights[l].Image) {
        Distance = 0;
        Falloff = false;
        int x = floor(X - Env->Lights[l].Data.xOffset);
        int y = floor(Y - Env->Lights[l].Data.yOffset);
        LightColor = Env->Lights[l].Image->getPixelRolloffNO(x, y);
        LightColor = ScaleAlpha(LightColor, Env->Lights[l].Color[::Alpha]);
        Obscured = (LightColor[::Alpha] == 0);
        LightColor = Premultiply(LightColor);
        if (Obscured) goto found;
      } else if (Falloff) {
        XDistance = (abs(LightRay.End.X - LightRay.Start.X) * DistanceMultiplier);
        YDistance = (abs(LightRay.End.Y - LightRay.Start.Y) * DistanceMultiplier);
        Distance = PythagorasLookup(ClipByte(XDistance), ClipByte(YDistance));
        LightColor = Env->Lights[l].Color;
      } else {
        LightColor = Env->Lights[l].Color;
        Distance = 0;
      }
      if (Distance < 255) {
        Obscured = false;
        if (Env->Lights[l].Image) {
        } else if ((Env->Lights[l].Spread < 180.0f) && (Distance > 0)) {
          // directional
          float a_l = NormalizeAngle(AngleBetween(LightRay.Start, LightRay.End));
          float a_s = Env->Lights[l].Data.angleStart;
          float a_e = Env->Lights[l].Data.angleEnd;
          if (a_e < a_s) {
            float a_d = a_s - a_e;
            a_l = NormalizeAngle(a_l + a_d);
            a_s = NormalizeAngle(a_s + a_d);
            a_e = NormalizeAngle(a_e + a_d);
            if ((a_l < a_s) || (a_l > a_e)) Obscured = true;
          } else {
            if ((a_l < a_s) || (a_l > a_e)) Obscured = true;
          }
        }
        if (Obscured) goto found;

		    for (my = my1; my <= my2; my++) {
			    for (mx = mx1; mx <= mx2; mx++) {
				    Sector = Env->Matrix->getSector(mx, my);
            if (LightRay.intersect(Sector->Obstructions, IntersectionPoint, 4.0f)) {
              Obscured = true;
              goto found;
            }
			    }
		    }

        if (Env->Sprites != Null) {
          ListSpriteIterator sprites(Env->Sprites);
          FPoint lp = FPoint(Env->Lights[l].X, Env->Lights[l].Y);
          Sprite = sprites.current();
          while (Sprite) {
            if (Sprite == IgnoreSprite) {
              Sprite = sprites.next(); 
              continue;
            }
            if (Sprite == Env->Lights[l].Attached) {
              Sprite = sprites.next(); 
              continue;
            }
            FPoint p = FPoint(Sprite->Position.X, Sprite->Position.Y);
            if (p.distance2(&lp) > (ldist11)) {
              Sprite = sprites.next(); 
              continue;
            }
            FPoint vl = FLine(p, lp).vector(), vr;
            float d = vl.length2();
            switch (Sprite->Params.SpecialFX) {
            default:
              Sprite = sprites.next();
              continue;
              break;
            case fxCastShadow:
            case fxCastGraphicShadow:
              if (Falloff) {
                if (d > ldist11) {
                  Sprite = sprites.next();
                  continue;
                }
              }
              break;
            }
            d = sqrt(d);
            float sdist = Sprite->ZHeight;
            if (sdist <= 0.0f) sdist = 10000.0f;
            sdist *= sdist;
            float s = (Sprite->Obstruction.W + Sprite->Obstruction.H) / 4.0f;
            float h = Sprite->Obstruction.H / 2.0f;
            vl /= d;
            vl *= s;
            vr = vl.rotate90r();
            vl = vl.rotate90l();
            p.Y -= h;
            SpriteLine.Start = p;
            SpriteLine.End = p;
            SpriteLine.Start += vl;
            SpriteLine.End += vr;
            if (LightRay.intersect(SpriteLine, IntersectionPoint)) {
              float d = IntersectionPoint.distance2(&(LightRay.End));
              if (d < 4.0f) {
              } else {
                if (d < sdist) {
                  Obscured = true;
                  break;
                }
              }
            }
            Sprite = sprites.next();
          }
        }
		  found:

        if (!Obscured) {
          if ((Falloff) && (Distance > 0)) {
            aDistance = AlphaLevelLookup(Distance ^ 0xFF);
            Color[::Blue] = ClipByteHigh(Color[::Blue] + AlphaFromLevel(aDistance, LightColor[::Blue]));
            Color[::Green] = ClipByteHigh(Color[::Green] + AlphaFromLevel(aDistance, LightColor[::Green]));
            Color[::Red] = ClipByteHigh(Color[::Red] + AlphaFromLevel(aDistance, LightColor[::Red]));
          } else {
            Color[::Blue] = ClipByteHigh(Color[::Blue] + LightColor[::Blue]);
            Color[::Green] = ClipByteHigh(Color[::Green] + LightColor[::Green]);
            Color[::Red] = ClipByteHigh(Color[::Red] + LightColor[::Red]);
          }
        }
      }
    }
  }
  return Color;
}

int Lighting::RaycastStrip(Lighting::Environment *Env, float X1, float Y1, float X2, float Y2, SpriteParam *IgnoreSprite, bool EnableCulling, int Count, Pixel* pOut) {
  Pixel Color = Env->AmbientLight;
  Pixel LightColor;
  FLine LightRay;
  FPoint IntersectionPoint;
  AlphaLevel *aDistance;
  SpriteParam *Sprite;
  FLine SpriteLine;
  int XDistance, YDistance;
  int Distance;
  float DistanceMultiplier = 0;
  float ldist, ldist11;
  bool Obscured, Falloff;
  int mx1 = 0, my1 = 0, mx2 = 0, my2 = 0, mx = 0, my = 0;
  Lighting::Sector *Sector = Null;
  std::vector<Lighting::Obstruction>::const_iterator Obstruction;
  float X = X1, Y = Y1;
  int Length = floor(FPoint(X1, Y1).distance(FPoint(X2, Y2)));
  float Xd = (X2 - X1) / (float)Length;
  float Yd = (Y2 - Y1) / (float)Length;
  if (Length > Count) Length = Count;
  if (Env->LightCount < 1) {
    _Fill(pOut, Color, Length);
    return 0;
  }
  float fsw = (float)Env->Matrix->SectorWidth;
  float fsh = (float)Env->Matrix->SectorHeight;
  for (int pixels = 0; pixels < Length; pixels++) {
    Color = Env->AmbientLight;
    for (int l = 0; l < Env->LightCount; ++l) {
      Obscured = false;
	    if (((!Env->Lights[l].Culled) || (!Env->Lights[l].PlaneCulled) || (!EnableCulling)) && (Env->Lights[l].Visible)) {
        LightColor = Pixel(0, 0, 0, 0);
        ldist = Env->Lights[l].Data.ldist;
        ldist11 = Env->Lights[l].Data.ldist11;
        Falloff = Env->Lights[l].FalloffDistance > 0;
        if (Falloff) {
          DistanceMultiplier = (255.0F / ldist);
        }
        LightRay.Start.X = Env->Lights[l].X;
        LightRay.Start.Y = Env->Lights[l].Y;
        LightRay.End.X = X;
        LightRay.End.Y = Y;
	      mx1 = ClipValue(floor(_Min(LightRay.Start.X, LightRay.End.X) / fsw), Env->Matrix->Width - 1);
  	    my1 = ClipValue(floor(_Min(LightRay.Start.Y, LightRay.End.Y) / fsh), Env->Matrix->Height - 1);
	      mx2 = ClipValue(ceil(_Max(LightRay.Start.X, LightRay.End.X) / fsw), Env->Matrix->Width - 1);
	      my2 = ClipValue(ceil(_Max(LightRay.Start.Y, LightRay.End.Y) / fsh), Env->Matrix->Height - 1);
        if (Env->Lights[l].Image) {
          Distance = 0;
          Falloff = false;
          int x = floor(X - Env->Lights[l].Data.xOffset);
          int y = floor(Y - Env->Lights[l].Data.yOffset);
          LightColor = Env->Lights[l].Image->getPixelRolloffNO(x, y);
          LightColor = ScaleAlpha(LightColor, Env->Lights[l].Color[::Alpha]);
          Obscured = (LightColor[::Alpha] == 0);
          LightColor = Premultiply(LightColor);
          if (Obscured) goto found;
        } else if (Falloff) {
          XDistance = (abs(LightRay.End.X - LightRay.Start.X) * DistanceMultiplier);
          YDistance = (abs(LightRay.End.Y - LightRay.Start.Y) * DistanceMultiplier);
          Distance = PythagorasLookup(ClipByte(XDistance), ClipByte(YDistance));
          LightColor = Env->Lights[l].Color;
        } else {
          LightColor = Env->Lights[l].Color;
          Distance = 0;
        }
        if (Distance < 255) {
          Obscured = false;
          if (Env->Lights[l].Image) {
          } else if ((Env->Lights[l].Spread < 180.0f) && (Distance > 0)) {
            // directional
            float a_l = NormalizeAngle(AngleBetween(LightRay.Start, LightRay.End));
            float a_s = Env->Lights[l].Data.angleStart;
            float a_e = Env->Lights[l].Data.angleEnd;
            if (a_e < a_s) {
              float a_d = a_s - a_e;
              a_l = NormalizeAngle(a_l + a_d);
              a_s = NormalizeAngle(a_s + a_d);
              a_e = NormalizeAngle(a_e + a_d);
              if ((a_l < a_s) || (a_l > a_e)) Obscured = true;
            } else {
              if ((a_l < a_s) || (a_l > a_e)) Obscured = true;
            }
          }
          if (Obscured) goto found;

		      for (my = my1; my <= my2; my++) {
			      for (mx = mx1; mx <= mx2; mx++) {
				      Sector = Env->Matrix->getSector(mx, my);
              if (LightRay.intersect(Sector->Obstructions, IntersectionPoint, 4.0f)) {
                Obscured = true;
                goto found;
              }
			      }
		      }

          if (Env->Sprites != Null) {
            FPoint lp = FPoint(Env->Lights[l].X, Env->Lights[l].Y);
            ListSpriteIterator sprites(Env->Sprites);
            Sprite = sprites.current();
            while (Sprite) {
              if (Sprite == IgnoreSprite) {
                Sprite = sprites.next(); 
                continue;
              }
              if (Sprite == Env->Lights[l].Attached) {
                Sprite = sprites.next(); 
                continue;
              }
              FPoint p = FPoint(Sprite->Position.X, Sprite->Position.Y);
              if (p.distance2(&lp) > (ldist11)) {
                Sprite = sprites.next(); 
                continue;
              }
              FPoint vl = FLine(p, lp).vector(), vr;
              float d = vl.length2();
              switch (Sprite->Params.SpecialFX) {
              default:
                Sprite = sprites.next();
                continue;
                break;
              case fxCastShadow:
              case fxCastGraphicShadow:
                if (Falloff) {
                  if (d > ldist11) {
                    Sprite = sprites.next();
                    continue;
                  }
                }
                break;
              }
              d = sqrt(d);
              float sdist = Sprite->ZHeight;
              if (sdist <= 0.0f) sdist = 10000.0f;
              sdist *= sdist;
              float s = (Sprite->Obstruction.W + Sprite->Obstruction.H) / 4.0f;
              float h = Sprite->Obstruction.H / 2.0f;
              vl /= d;
              vl *= s;
              vr = vl.rotate90r();
              vl = vl.rotate90l();
              p.Y -= h;
              SpriteLine.Start = p;
              SpriteLine.End = p;
              SpriteLine.Start += vl;
              SpriteLine.End += vr;
              if (LightRay.intersect(SpriteLine, IntersectionPoint)) {
                float d = IntersectionPoint.distance2(&(LightRay.End));
                if (d < 4.0f) {
                } else {
                  if (d < sdist) {
                    Obscured = true;
                    break;
                  }
                }
              }
              Sprite = sprites.next();
            }
          }
		    found:

          if (!Obscured) {
            if ((Falloff) && (Distance > 0)) {
              aDistance = AlphaLevelLookup(Distance ^ 0xFF);
              Color[::Blue] = ClipByteHigh(Color[::Blue] + AlphaFromLevel(aDistance, LightColor[::Blue]));
              Color[::Green] = ClipByteHigh(Color[::Green] + AlphaFromLevel(aDistance, LightColor[::Green]));
              Color[::Red] = ClipByteHigh(Color[::Red] + AlphaFromLevel(aDistance, LightColor[::Red]));
            } else {
              Color[::Blue] = ClipByteHigh(Color[::Blue] + LightColor[::Blue]);
              Color[::Green] = ClipByteHigh(Color[::Green] + LightColor[::Green]);
              Color[::Red] = ClipByteHigh(Color[::Red] + LightColor[::Red]);
            }
          }
        }
      }
    }
    *pOut = Color;
    ++pOut;
    X += Xd;
    Y += Yd;
  }
  return Length;
}

Export int RenderLightingEnvironment(Lighting::Camera *Camera, Lighting::Environment *Environment) {
Lighting::LightSource *Light;
std::vector<Lighting::Obstruction>::iterator Obstruction;
Lighting::Plane *Plane;
Polygon<FPoint> ShadowPoly;
Polygon<GradientVertex> GradientShadowPoly;
GradientVertex ShadowVertex;
FPoint Point, LightPoint, LinePoint[4], LineCenter;
FPoint Vector;
Rectangle FillRect, LightRect, CameraRect, CacheRect, ScratchRect;
FRect LightFRect;
float LightDistance, Falloff;
SpriteParam *Sprite;
float w = 0, h = 0;
int mx1 = 0, my1 = 0, mx2 = 0, my2 = 0, mx = 0, my = 0;
Lighting::Sector *Sector = Null;
bool Ignore = false, Scaling = false;
int SpriteCount = 0;
int iCount = 0;
Pixel SavedColor, LightColor;
Image* RenderTarget;
Image* NormalMap;
int iOffsetX = 0, iOffsetY = 0;
float OffsetX = 0, OffsetY = 0;
int LightIterations = 0;
float FuzzyOffset = 0, FlickerAmount = 0;
  if (!Camera) return Failure;
  if (!Environment) return Failure;
  if (!Camera->OutputBuffer) return Failure;
  if (!Camera->ScratchBuffer) return Failure;
  if (!Environment->Lights) return Failure;
  Camera->OutputBuffer->setClipRectangle(Camera->OutputRectangle);
  Camera->ScratchBuffer->setClipRectangle(Camera->OutputRectangle);

  CameraRect = Camera->OutputRectangle;
  RenderTarget = Camera->ScratchBuffer;
  iOffsetX = OffsetX = Camera->ScrollX;
  iOffsetY = OffsetY = Camera->ScrollY;

  if (Environment->LightCount < 1) return Trivial_Success;

  Scaling = (Camera->OutputScaleRatio != 1.0);

  for (int l = 0; l < Environment->LightCount; l++) {
    Environment->Lights[l].FalloffDistance2 = Environment->Lights[l].FalloffDistance * Environment->Lights[l].FalloffDistance;
    Environment->Lights[l].Culled = false;
  }

  for (int l = 0; l < Environment->LightCount; l++) {

    bool Light_Clipped = false;

    iOffsetX = OffsetX = Camera->ScrollX;
    iOffsetY = OffsetY = Camera->ScrollY;
    Light = &(Environment->Lights[l]);
  	Light->Culled = true;
    if (Light->Attached) {
      Light->Angle = Light->Attached->Velocity.B;
      Light->X = Light->Attached->Position.X + Light->AttachX + (sin(Light->Angle * Radian) * Light->AttachV) + (sin((Light->Angle + 90) * Radian) * Light->AttachH);
      Light->Y = Light->Attached->Position.Y + Light->AttachY + (-cos(Light->Angle * Radian) * Light->AttachV) + (-cos((Light->Angle + 90) * Radian) * Light->AttachH);
    }
    Light->Angle = NormalizeAngle(Light->Angle);
    if (Light->Image) {
      float w = (Light->Image->Width / 2.0f);
      float h = (Light->Image->Height / 2.0f);
      Light->Data.xOffset = Light->X - w - Light->ImageAlignX;
      Light->Data.yOffset = Light->Y - h - Light->ImageAlignY;
      Light->Data.ldist = _Max(w, h);
      Light->Data.ldist11 = pow(Light->Data.ldist * 1.1f, 2);
    } else {
      Light->Data.ldist = Light->FalloffDistance;
      Light->Data.ldist11 = pow(Light->FalloffDistance * 1.1f, 2);
      Light->Data.xOffset = Light->Data.yOffset = 0;
    }
    if (Light->Spread < 180.0f) {
      float ls = Light->Spread / 2.0f;
      Light->Data.angleStart = NormalizeAngle(Light->Angle - ls);
      Light->Data.angleEnd = NormalizeAngle(Light->Angle + ls);
    }
    if ((Light->Cache != Null) && (Light->CacheValid == false)) {
      CameraRect = Light->Cache->getRectangle();
      RenderTarget = Light->Cache;
      iOffsetX = OffsetX = Light->X - Light->FalloffDistance;
      iOffsetY = OffsetY = Light->Y - Light->FalloffDistance;
    } else {
      CameraRect = Camera->OutputRectangle;
      RenderTarget = Camera->ScratchBuffer;
    }
    RenderTarget->setClipRectangle(RenderTarget->getRectangle());
    if (Light->Visible) {
      LightColor = Light->Color;
      if (Light->FlickerLevel != 0) {
        FlickerAmount = rand() * Light->FlickerLevel / (float)(RAND_MAX);
        if ((Light->Cache != Null)) {
        } else {
          LightColor[::Alpha] = ClipByte(LightColor[::Alpha] - (FlickerAmount * LightColor[::Alpha]));
        }
      } else {
        FlickerAmount = 0;
      }
      if (Scaling) {
        LightPoint = FPoint((Light->X - OffsetX) * Camera->OutputScaleRatio, (Light->Y - OffsetY) * Camera->OutputScaleRatio);
        LightDistance = Light->FalloffDistance * 1.1 * Camera->OutputScaleRatio;
        Falloff = Light->FalloffDistance * Camera->OutputScaleRatio;
      } else {
        LightPoint = FPoint(Light->X - OffsetX, Light->Y - OffsetY);
        LightDistance = Light->FalloffDistance * 1.1;
        Falloff = Light->FalloffDistance;
      }

      if (Light->Image) {
        // bitmapped
        w = Light->Image->Width / 2.0f;
        h = Light->Image->Height / 2.0f;
        LightRect.setValuesAbsolute(LightPoint.X - floor(w) - Light->ImageAlignX, LightPoint.Y - floor(h) - Light->ImageAlignY, LightPoint.X + ceil(w) - Light->ImageAlignX, LightPoint.Y + ceil(h) - Light->ImageAlignY);
        ScratchRect = LightRect;
        LightFRect.X1 = LightPoint.X - w + OffsetX - Light->ImageAlignX;
        LightFRect.Y1 = LightPoint.Y - h + OffsetY - Light->ImageAlignY;
        LightFRect.X2 = LightPoint.X + w + OffsetX - Light->ImageAlignX;
        LightFRect.Y2 = LightPoint.Y + h + OffsetY - Light->ImageAlignY;
        Light_Clipped = !ClipRectangle_Rect(&LightRect, &CameraRect);

        if (!Light_Clipped) {
          // render the light's sphere of illumination
          RenderTarget->setClipRectangle(LightRect);
          FillRect.setValuesAbsolute(LightPoint.X - floor(w) - Light->ImageAlignX, LightPoint.Y - floor(h) - Light->ImageAlignY, LightPoint.X + ceil(w) - Light->ImageAlignX, LightPoint.Y + ceil(h) - Light->ImageAlignY);
          if ((Light->CacheValid) && (Light->Cache != Null)) {
          } else {
            BlitSimple_Normal(RenderTarget, Light->Image, &FillRect, 0, 0);
            //FilterSimple_Gradient_Radial(RenderTarget, &FillRect, Light->Color, Pixel(0, 0, 0, LightColor[::Alpha]));
          }
        }
      } else if (abs(Light->Spread) > 90) {
        // unidirectional
        if (Light->FalloffDistance <= 0) {
          // no falloff (fill)
          LightRect = CameraRect;
          ScratchRect = LightRect;
          LightFRect.X1 = LightRect.Left + OffsetX;
          LightFRect.Y1 = LightRect.Top + OffsetY;
          LightFRect.X2 = LightRect.right() + OffsetX;
          LightFRect.Y2 = LightRect.bottom() + OffsetY;
          Light_Clipped = false;
          Camera->ScratchBuffer->fill(LightColor);
        } else {
          // falloff (radial gradient)

          LightRect.setValuesAbsolute(LightPoint.X - Falloff, LightPoint.Y - Falloff, LightPoint.X + Falloff, LightPoint.Y + Falloff);
          ScratchRect = LightRect;
          LightFRect.X1 = LightRect.Left + OffsetX;
          LightFRect.Y1 = LightRect.Top + OffsetY;
          LightFRect.X2 = LightRect.right() + OffsetX;
          LightFRect.Y2 = LightRect.bottom() + OffsetY;
          Light_Clipped = !ClipRectangle_Rect(&LightRect, &CameraRect);

          if (!Light_Clipped) {
            // render the light's sphere of illumination
            float so = 0.0f;
            if (Light->LightSize > 0) 
              so = Light->LightSize / Light->FalloffDistance;
            RenderTarget->setClipRectangle(LightRect);
            FillRect.setValuesAbsolute(LightPoint.X - Falloff, LightPoint.Y - Falloff, LightPoint.X + Falloff, LightPoint.Y + Falloff);
            if ((Light->CacheValid) && (Light->Cache != Null)) {
            } else {
              FilterSimple_Gradient_Radial_Ex(RenderTarget, &FillRect, Light->Color, Pixel(0, 0, 0, LightColor[::Alpha]), so, 0.0f);
            }
          }
        }
      } else {
        // directional
        Camera->ScratchBuffer->setClipRectangle(Camera->ScratchBuffer->getRectangle());
        RenderTarget->fill(0);
        if (Light->FalloffDistance <= 0) {
          // no falloff (solid filled convex polygon extended to infinity)
          LightRect = CameraRect;
          ScratchRect = LightRect;
          LightFRect.X1 = LightRect.Left + OffsetX;
          LightFRect.Y1 = LightRect.Top + OffsetY;
          LightFRect.X2 = LightRect.right() + OffsetX;
          LightFRect.Y2 = LightRect.bottom() + OffsetY;
          Light_Clipped = false;

          ShadowPoly.Allocate(3);

          ShadowPoly.Append(LightPoint);

          Vector.X = sin((Light->Angle - (Light->Spread / 2)) * Radian);
          Vector.Y = -cos((Light->Angle - (Light->Spread / 2)) * Radian);
          Vector *= 10000;
          ShadowPoly.Append(FPoint(LightPoint.X + Vector.X, LightPoint.Y + Vector.Y));

          Vector.X = sin((Light->Angle + (Light->Spread / 2)) * Radian);
          Vector.Y = -cos((Light->Angle + (Light->Spread / 2)) * Radian);
          Vector *= 10000;
          ShadowPoly.Append(FPoint(LightPoint.X + Vector.X, LightPoint.Y + Vector.Y));

          ShadowPoly.Finalize();

          Camera->ScratchBuffer->setClipRectangle(LightRect);
          FilterSimple_ConvexPolygon(Camera->ScratchBuffer, &ShadowPoly, LightColor, Null, 0);
        } else {
          // falloff (gradient filled convex polygon extended to FalloffDistance pixels away)
          LightIterations = Light->Fuzziness;
          FuzzyOffset = 0;

fuzzyrender:
          GradientShadowPoly.Allocate(3);
          ShadowVertex.X = LightPoint.X;
          ShadowVertex.Y = LightPoint.Y;
          if ((Light->Fuzziness > 0) && (FuzzyOffset > 0)) {
            int fa = (255 / Light->Fuzziness);
            ShadowVertex.Color = Premultiply(ScaleAlpha<Pixel>(LightColor, ClipByte(fa))).V;
          } else {
            ShadowVertex.Color = LightColor.V;
          }
          GradientShadowPoly.Append(ShadowVertex);

          Vector.X = sin((Light->Angle - (Light->Spread / 2) + (FuzzyOffset * 2.5)) * Radian);
          Vector.Y = -cos((Light->Angle - (Light->Spread / 2) + (FuzzyOffset * 2.5)) * Radian);
          Vector *= (Light->FalloffDistance * Camera->OutputScaleRatio + FuzzyOffset);
          ShadowVertex.X = LightPoint.X + Vector.X;
          ShadowVertex.Y = LightPoint.Y + Vector.Y;
          ShadowVertex.Color = Pixel(0, 0, 0, LightColor[::Alpha]).V;
          GradientShadowPoly.Append(ShadowVertex);

          Vector.X = sin((Light->Angle + (Light->Spread / 2) - (FuzzyOffset * 2.5)) * Radian);
          Vector.Y = -cos((Light->Angle + (Light->Spread / 2) - (FuzzyOffset * 2.5)) * Radian);
          Vector *= (Light->FalloffDistance * Camera->OutputScaleRatio + FuzzyOffset);
          ShadowVertex.X = LightPoint.X + Vector.X;
          ShadowVertex.Y = LightPoint.Y + Vector.Y;
          ShadowVertex.Color = Pixel(0, 0, 0, LightColor[::Alpha]).V;
          GradientShadowPoly.Append(ShadowVertex);

          GradientShadowPoly.Finalize();

          if (FuzzyOffset == 0) {
            LightRect.setValuesAbsolute(GradientShadowPoly.MinimumX(), GradientShadowPoly.MinimumY(), GradientShadowPoly.MaximumX(), GradientShadowPoly.MaximumY());
            ScratchRect = LightRect;
            LightFRect.X1 = LightRect.Left + OffsetX;
            LightFRect.Y1 = LightRect.Top + OffsetY;
            LightFRect.X2 = LightRect.right() + OffsetX;
            LightFRect.Y2 = LightRect.bottom() + OffsetY;
            Light_Clipped = !ClipRectangle_Rect(&LightRect, &CameraRect);
          }

          if (!Light_Clipped) {
            if (FuzzyOffset == 0) RenderTarget->setClipRectangle(LightRect);
            if ((Light->CacheValid) && (Light->Cache != Null)) {
              LightRect.setValuesAbsolute(GradientShadowPoly.MinimumX(), GradientShadowPoly.MinimumY(), GradientShadowPoly.MaximumX(), GradientShadowPoly.MaximumY());
              CacheRect = LightRect;
              CacheRect.Left = CacheRect.Top = 0;
              BlitSimple_Normal(Camera->ScratchBuffer, Light->Cache, &LightRect, 0, 0);
//              FilterSimple_Fill(RenderTarget, &LightRect, Pixel(0, 0, 0, 255));
            } else {
              FilterSimple_ConvexPolygon_Gradient(RenderTarget, &GradientShadowPoly, RenderFunction_Additive, 0);
              if (LightIterations > 0) {
                FuzzyOffset += 1;
                LightIterations--;
                goto fuzzyrender;
              }
            }
          }
        }
      }


	    Light->Culled = Light_Clipped;
      Light->Rect.setValues(0, 0, 0, 0);
      if (!Light_Clipped) {

	      mx1 = ClipValue((Light->X - Light->FalloffDistance) / (float)Environment->Matrix->SectorWidth, Environment->Matrix->Width - 1);
	      my1 = ClipValue((Light->Y - Light->FalloffDistance) / (float)Environment->Matrix->SectorHeight, Environment->Matrix->Height - 1);
	      mx2 = ClipValue(ceil((Light->X + Light->FalloffDistance) / (float)Environment->Matrix->SectorWidth), Environment->Matrix->Width - 1);
	      my2 = ClipValue(ceil((Light->Y + Light->FalloffDistance) / (float)Environment->Matrix->SectorHeight), Environment->Matrix->Height - 1);
        Light->Rect.setValues(_Min(mx1, mx2), _Min(my1, my2), _Max(mx1, mx2) - _Min(mx1, mx2), _Max(my1, my2) - _Min(my1, my2));

        ShadowPoly.Allocate(4);
        ShadowPoly.SetCount(4);


        if ((Environment->Matrix) && !(Light->CacheValid && (Light->Cache != Null))) {
			  float vs = 0;
			for (my = my1; my <= my2; my++) {
				for (mx = mx1; mx <= mx2; mx++) {
					Sector = Environment->Matrix->getSector(mx, my);
					for (Obstruction = Sector->Obstructions.begin(); Obstruction != Sector->Obstructions.end(); ++Obstruction) {

						LinePoint[0] = Obstruction->Line.Start;
						LinePoint[1] = Obstruction->Line.End;
						LinePoint[0].X -= OffsetX;
						LinePoint[1].X -= OffsetX;
						LinePoint[0].Y -= OffsetY;
						LinePoint[1].Y -= OffsetY;
						if (Scaling) {
							LinePoint[0].X *= Camera->OutputScaleRatio;
							LinePoint[0].Y *= Camera->OutputScaleRatio;
							LinePoint[1].X *= Camera->OutputScaleRatio;
							LinePoint[1].Y *= Camera->OutputScaleRatio;
						}

						Vector = FPoint(LightPoint, LinePoint[0]);
						vs = (sqrt((Vector.X * Vector.X) + (Vector.Y * Vector.Y)));
						Vector.X = afloor(Vector.X / vs);
						Vector.Y = afloor(Vector.Y / vs);
						ShadowPoly.SetVertex(0, FPoint(LinePoint[0].X + Vector.X, LinePoint[0].Y + Vector.Y));

						Vector = FPoint(LightPoint, LinePoint[0]);
						Vector *= 10000;
						ShadowPoly.SetVertex(1, FPoint(LinePoint[0].X + Vector.X, LinePoint[0].Y + Vector.Y));

						Vector = FPoint(LightPoint, LinePoint[1]);
						Vector *= 10000;
						ShadowPoly.SetVertex(2, FPoint(LinePoint[1].X + Vector.X, LinePoint[1].Y + Vector.Y));

						Vector = FPoint(LightPoint, LinePoint[1]);
						vs = (sqrt((Vector.X * Vector.X) + (Vector.Y * Vector.Y)));
						Vector.X = afloor(Vector.X / vs);
						Vector.Y = afloor(Vector.Y / vs);
						ShadowPoly.SetVertex(3, FPoint(LinePoint[1].X + Vector.X, LinePoint[1].Y + Vector.Y));

            if (Camera->AntiAlias) {
						  FilterSimple_ConvexPolygon_AntiAlias(RenderTarget, &ShadowPoly, Pixel(0,0,0,255), RenderFunction_Shadow, 0);
            } else {
						  FilterSimple_ConvexPolygon(RenderTarget, &ShadowPoly, Pixel(0,0,0,255), Null, 0);
            }
												
					}
				}
			}
        }

        if ((Light->Cache != Null)) {
          LightRect.Left = Light->X - Light->FalloffDistance - Camera->ScrollX;
          LightRect.Top = Light->Y - Light->FalloffDistance - Camera->ScrollY;
          LightRect.setRight(Light->X + Light->FalloffDistance - Camera->ScrollX);
          LightRect.setBottom(Light->Y + Light->FalloffDistance - Camera->ScrollY);
          CacheRect = Light->Cache->getRectangle();
          Camera->ScratchBuffer->setClipRectangle(LightRect);
          if (FlickerAmount != 0) {
            BlitSimple_Normal_Gamma(Camera->ScratchBuffer, Light->Cache, &LightRect, 0, 0, 255 - (255 * FlickerAmount));
          } else {
            BlitSimple_Normal(Camera->ScratchBuffer, Light->Cache, &LightRect, 0, 0);
          }
          ScratchRect = LightRect;
        }
        LightPoint.X += OffsetX;
        LightPoint.Y += OffsetY;
        CameraRect = Camera->OutputRectangle;
        RenderTarget = Camera->ScratchBuffer;
        OffsetX = Camera->ScrollX;
        OffsetY = Camera->ScrollY;
        LightPoint.X -= OffsetX;
        LightPoint.Y -= OffsetY;
  
        Sprite = Environment->Sprites;
        SpriteCount = 0;
        while (Sprite) {
          SpriteCount++;
          if (Sprite->Visible) {
            switch (Sprite->Params.SpecialFX) {
            default:
              Ignore = true;
              break;
            case fxCastShadow:
            case fxCastGraphicShadow:
              FRect SpriteRect = Sprite->getRect();
              if (SpriteRect.intersect(&LightFRect)) {
                Ignore = false;
              } else {
                Ignore = true;
              }
              break;
            }
            if (Sprite == Light->Attached) Ignore = true;

            if (!Ignore) {
              float s = (Sprite->Obstruction.W + Sprite->Obstruction.H) / 4;
              float h = Sprite->Obstruction.H / 2;
              float a = Radians(AngleBetween(Sprite->Position, *Light));
              float mul = Sprite->ZHeight;
              if (mul <= 0) mul = 10000;
              LinePoint[0].X = (sin(a - Radians(90)) * s) + Sprite->Position.X;
              LinePoint[0].Y = (-cos(a - Radians(90)) * s) + Sprite->Position.Y - h;
              LinePoint[1].X = (sin(a + Radians(90)) * s) + Sprite->Position.X;
              LinePoint[1].Y = (-cos(a + Radians(90)) * s) + Sprite->Position.Y - h;
              LinePoint[0].X -= OffsetX;
              LinePoint[1].X -= OffsetX;
              LinePoint[0].Y -= OffsetY;
              LinePoint[1].Y -= OffsetY;
              if (Scaling) {
                LinePoint[0].X *= Camera->OutputScaleRatio;
                LinePoint[0].Y *= Camera->OutputScaleRatio;
                LinePoint[1].X *= Camera->OutputScaleRatio;
                LinePoint[1].Y *= Camera->OutputScaleRatio;
              }
              ShadowPoly.SetVertex(0, LinePoint[0]);

              Vector = FPoint(LightPoint, LinePoint[0]);
              Vector *= mul;
              ShadowPoly.SetVertex(1, FPoint(LinePoint[0].X + Vector.X, LinePoint[0].Y + Vector.Y));

              Vector = FPoint(LightPoint, LinePoint[1]);
              Vector *= mul;
              ShadowPoly.SetVertex(2, FPoint(LinePoint[1].X + Vector.X, LinePoint[1].Y + Vector.Y));

              ShadowPoly.SetVertex(3, LinePoint[1]);

              if (Camera->AntiAlias) {
                FilterSimple_ConvexPolygon_AntiAlias(Camera->ScratchBuffer, &ShadowPoly, Pixel(0,0,0,255), RenderFunction_Shadow, 0);
              } else {
                FilterSimple_ConvexPolygon(Camera->ScratchBuffer, &ShadowPoly, Pixel(0,0,0,255), Null, 0);
              }
            }
          }

          Sprite = Sprite->pNext;
        }


        if ((Light->Cache != Null)) {
          Light->CacheValid = 1;
        }
        if (!Light_Clipped) {
          if (Camera->SaturationMode == 1) {
            BlitSimple_Screen_Opacity(Camera->OutputBuffer, Camera->ScratchBuffer, &ScratchRect, (ScratchRect.Left - Camera->OutputRectangle.Left), (ScratchRect.Top - Camera->OutputRectangle.Top), Light->Color[::Alpha]);
          } else {
            BlitSimple_Additive_Opacity(Camera->OutputBuffer, Camera->ScratchBuffer, &ScratchRect, (ScratchRect.Left - Camera->OutputRectangle.Left), (ScratchRect.Top - Camera->OutputRectangle.Top), Light->Color[::Alpha]);
          }
        }
      }
    }
  }

  iOffsetX = OffsetX = Camera->ScrollX;
  iOffsetY = OffsetY = Camera->ScrollY;

  Sprite = Environment->Sprites;
  while (Sprite) {
    if (Sprite->Visible) {
      Sprite->Params.IlluminationLevel = Raycast(Environment, Sprite->Position.X, Sprite->Position.Y, Sprite, false);
    } else {
      Sprite->Params.IlluminationLevel = Environment->AmbientLight;      
    }
    Sprite = Sprite->pNext;
  }

  Lighting::sort_entry *SortEntries = Null;
  Lighting::sort_entry *FirstSortEntry = Null;
  int SortEntryCount = 0, SortBufferCount = 0;
  {
    int y2 = 0;
    SortBufferCount = Environment->PlaneCount + SpriteCount + 1;
    SortEntries = StaticAllocate<Lighting::sort_entry>(ListBuffer, SortBufferCount + 256) + 32;
	  _Fill<Byte>(SortEntries, 0, sizeof(Lighting::sort_entry) * SortBufferCount);
    if (Environment->Planes) {
      Rectangle rctWall, rctTop;
      for (int i = 0; i < Environment->PlaneCount; i++) {
        {
          Plane = &(Environment->Planes[i]);
          rctWall = Plane->fullRect();
          y2 = rctWall.bottom();
          rctWall.translate(-iOffsetX, -iOffsetY);
          if (ClipRectangle_Rect(&rctWall, &CameraRect)) {
            SortEntries[SortEntryCount].type = Lighting::plane;
            SortEntries[SortEntryCount].y = y2;
            SortEntries[SortEntryCount].Plane = Plane;
            SortEntries[SortEntryCount].pNext = &(SortEntries[SortEntryCount + 1]);
            SortEntryCount++;
          }
        }
      }
    }
    if (Environment->Sprites) {
      Rectangle rctSprite;
      Sprite = Environment->Sprites;
      while (Sprite) {
        if ((Sprite->Visible) && (Sprite->Params.DiffuseLight)) {
          if (Sprite->Params.Alpha != 0) {
            rctSprite = Sprite->getRectangle();
            y2 = rctSprite.bottom();
            rctSprite.translate(-iOffsetX, -iOffsetY);
            if (ClipRectangle_Rect(&rctSprite, &CameraRect)) {
              SortEntries[SortEntryCount].type = Lighting::sprite;
              SortEntries[SortEntryCount].y = y2 + Sprite->Position.Z;
              SortEntries[SortEntryCount].Sprite = Sprite;
              SortEntries[SortEntryCount].pNext = &(SortEntries[SortEntryCount + 1]);
              SortEntryCount++;
            }
          }
        }
        Sprite = Sprite->pNext;
      }
    }
    SortEntries[SortEntryCount - 1].pNext = Null;
    FirstSortEntry = SortLinkedList<Lighting::sort_entry>(&(SortEntries[0]));
  }

  Lighting::sort_entry *SortEntry = FirstSortEntry;
  {
    Rectangle rctDest, rctSource, rctCopy;
    Rectangle rctWall, rctTop, rctFullWall;
    float x = 0, y = 0, s = 0, r = 0, x2 = 0;
  	int xo = 0, xo2 = 0;
    DoubleWord n = 0;
    bool scaled = false, rotated = false;
    DoubleWord iTemp = 0;

    TexturedPolygon PlanePoly;
    Image* PlaneTexture;
    FPoint TexPoint[2];
    PlanePoly.Allocate(4);
    PlanePoly.SetCount(4);
//    PlaneTexture = new Image(RenderTarget->Width, 1); 
    PlaneTexture = new Image(32, 1); 

    while (SortEntry) {
      if (SortEntry->type == Lighting::sprite) {
        Sprite = SortEntry->Sprite;
        NormalMap = 0;
        for (int si = 0; si < Sprite->Graphic.SecondaryImageCount; si++) {
          if (Sprite->Graphic.pSecondaryImages[si].ImageType == siNormalMap) {
            NormalMap = Sprite->Graphic.pSecondaryImages[si].pImage;
            break;
          }
        }
        SavedColor = Sprite->Params.IlluminationLevel;
        SavedColor[::Alpha] = 255;
        w = Sprite->Graphic.Rectangle.Width / 2; h = Sprite->Graphic.Rectangle.Height;
        x = Sprite->Position.X - OffsetX - (Sprite->Graphic.XCenter - w);
        y = Sprite->Position.Y - OffsetY + (h) - Sprite->Graphic.YCenter;
        s = Sprite->Params.Scale; r = Sprite->Params.Angle;
        scaled = (s != 1); rotated = (((int)r) % 360) != 0;
        if ((!scaled) && (!rotated)) {
          rctDest.Left = ceil(x - (w)); rctDest.Top = ceil(y - h);
          rctDest.Width = Sprite->Graphic.Rectangle.Width; rctDest.Height = Sprite->Graphic.Rectangle.Height;
          rctSource = Sprite->Graphic.Rectangle;
          if (Clip2D_PairToRect(&rctDest, &rctSource, &(Camera->OutputRectangle))) {
            if (NormalMap) {
              SavedColor = Environment->AmbientLight;
              SavedColor[::Alpha] = 255;
              switch(Sprite->Params.BlitMode) {
              case 0:
                iTemp = Sprite->Graphic.pImage->MatteColor.V;
                Sprite->Graphic.pImage->MatteColor = Sprite->Graphic.MaskColor;
                BlitSimple_Matte_Tint_Opacity(Camera->OutputBuffer, Sprite->Graphic.pImage, &rctDest, rctSource.Left, rctSource.Top, SavedColor, abs(Sprite->Params.Alpha) * 255);
                Sprite->Graphic.pImage->MatteColor.V = iTemp;
                break;
              case 1:
                BlitSimple_SourceAlpha_Tint_Opacity(Camera->OutputBuffer, Sprite->Graphic.pImage, &rctDest, rctSource.Left, rctSource.Top, SavedColor, abs(Sprite->Params.Alpha) * 255);
                break;
              }
              for (int si = 0; si < Sprite->Graphic.SecondaryImageCount; si++) {
                switch (Sprite->Graphic.pSecondaryImages[si].ImageType) {
                case siNormalMap:
                  for (int l = 0; l < Environment->LightCount; l++) {
                    if (Environment->Lights[l].Culled) { 
                    } else {
                      bool obscured = SightCheck(Environment, Environment->Lights[l].X, Environment->Lights[l].Y, Sprite->Position.X, Sprite->Position.Y, Sprite, Environment->Lights[l].Attached) != 1;
                      if (obscured) {
                      } else {
                        FPoint3 LightVector = FPoint3(FPoint3(Environment->Lights[l].X, Environment->Lights[l].Y), FPoint3(Sprite->Position.X, Sprite->Position.Y, Sprite->Position.Z));
                        Pixel LightColor;
                        if (Environment->Lights[l].Image) {
                          float w = Environment->Lights[l].Image->Width / 2;
                          float h = Environment->Lights[l].Image->Height / 2;
                          int x = floor(Sprite->Position.X - Environment->Lights[l].X + w + Environment->Lights[l].ImageAlignX);
                          int y = floor(Sprite->Position.Y - Environment->Lights[l].Y + h + Environment->Lights[l].ImageAlignY);
                          LightColor = Environment->Lights[l].Image->getPixelClipNO(x, y);
                          LightColor = ScaleAlpha(LightColor, Environment->Lights[l].Color[::Alpha]);
                        } else if (Environment->Lights[l].FalloffDistance > 0) {
                          Byte a = 255;
                          int d = LightVector.length() * 255;
                          a = 255 - ClipByte(d / Environment->Lights[l].FalloffDistance);
                          LightColor = Environment->Lights[l].Color;
                          LightColor[::Alpha] = a;
                        }
                        BlitSimple_NormalMap_Additive_SourceAlpha(Camera->OutputBuffer, Sprite->Graphic.pSecondaryImages[si].pImage, &rctDest, rctSource.Left, rctSource.Top, &LightVector, LightColor);
                        iCount++;
                      }
                    }
                  }
                  break;
                case siGlowMap:
                  BlitSimple_Additive(Camera->OutputBuffer, Sprite->Graphic.pSecondaryImages[si].pImage, &rctDest, rctSource.Left, rctSource.Top);
                  iCount++;
                  break;
                case siShadowMap:
                  BlitSimple_Subtractive(Camera->OutputBuffer, Sprite->Graphic.pSecondaryImages[si].pImage, &rctDest, rctSource.Left, rctSource.Top);
                  iCount++;
                  break;
                case siLightMap:
                  BlitSimple_Lightmap_RGB(Camera->OutputBuffer, Sprite->Graphic.pSecondaryImages[si].pImage, &rctDest, rctSource.Left, rctSource.Top);
                  iCount++;
                  break;
                default:
                  break;
                }
              }
            } else {
              switch(Sprite->Params.BlitMode) {
              default: case 0:
                iTemp = Sprite->Graphic.pImage->MatteColor.V;
                Sprite->Graphic.pImage->MatteColor = Sprite->Graphic.MaskColor;
                BlitSimple_Matte_Tint_Opacity(Camera->OutputBuffer, Sprite->Graphic.pImage, &rctDest, rctSource.Left, rctSource.Top, SavedColor, abs(Sprite->Params.Alpha) * 255);
                iCount++;
                Sprite->Graphic.pImage->MatteColor.V = iTemp;
                break;
              case 1:
                BlitSimple_SourceAlpha_Tint_Opacity(Camera->OutputBuffer, Sprite->Graphic.pImage, &rctDest, rctSource.Left, rctSource.Top, SavedColor, abs(Sprite->Params.Alpha) * 255);
                iCount++;
                break;
              case 7:
                if (Camera->SaturationMode == 1) {
                  BlitSimple_Screen_Opacity(Camera->OutputBuffer, Sprite->Graphic.pImage, &rctDest, rctSource.Left, rctSource.Top, abs(Sprite->Params.Alpha) * 255);
                  iCount++;
                } else {
                  BlitSimple_Additive_Opacity(Camera->OutputBuffer, Sprite->Graphic.pImage, &rctDest, rctSource.Left, rctSource.Top, abs(Sprite->Params.Alpha) * 255);
                  iCount++;
                }
              case 2: case 3: case 4: case 5: case 6: case 8:
                  break;
              }
            }
          }
        }
      } else if (SortEntry->type == Lighting::plane) {
        Plane = SortEntry->Plane;
        rctTop = Plane->topRect();
        rctWall = Plane->bottomRect();
        rctTop.translate(-iOffsetX, -iOffsetY);
        rctWall.translate(-iOffsetX, -iOffsetY);
        rctFullWall = rctWall;
    		x = rctWall.Left;
        x2 = rctWall.right();
		    ClipRectangle_Image(&rctTop, Camera->OutputBuffer);
		    ClipRectangle_Image(&rctWall, Camera->OutputBuffer);
		    xo = (rctWall.Left - x);
        xo2 = (x2 - rctWall.right());
		    if (Plane->Height != 0) FilterSimple_Fill(Camera->OutputBuffer, &rctTop, Environment->AmbientLight);
          if ((rctWall.Width > 0) && (rctWall.Height > 0)) {
            if (rctWall.Width + 16 > PlaneTexture->Width) {
              PlaneTexture->resize(rctWall.Width + 16, 1);
            }
            rctSource.Width = rctWall.Width;
            rctSource.Height = 1;
            rctSource.Left = 0;
            rctSource.Top = 0;
            float y = _Max<float>(Plane->Start.Y, Plane->End.Y);
            float ex = _Max(Plane->Start.X, Plane->End.X);
            float sx = _Min(Plane->Start.X, Plane->End.X);
		        Pixel color = Pixel(0, 0, 0, 255);
            Pixel *pDest = PlaneTexture->pointer(0, 0);
		        int i = 0, imax = rctWall.Width;
            for (int l = 0; l < Environment->LightCount; l++) {
              if (Environment->Lights[l].Image) {
                Environment->Lights[l].PlaneCulled = false;
              } else {
                Environment->Lights[l].PlaneCulled = !(Environment->Lights[l].Rect.intersect(rctWall));
              }
            }
            RaycastStrip(Environment, sx + xo, y, ex, y, Null, true, imax, pDest);
           // for (float x = sx; x <= ex; x += 1.0f) {
      			  //if (i >= imax) break;
			        //color = Raycast(Environment, x + xo, y, Null, true);
           //   *pDest = color;
           //   pDest++;
           //   i++;
           // }
            PlaneTexture->dirty();
            int pw = PlaneTexture->Width;
            PlaneTexture->Width = rctSource.Width;
            BlitResample_Normal(Camera->OutputBuffer, PlaneTexture, &rctWall, &rctSource, SampleRow_Linear);
            iCount++;
            PlaneTexture->Width = pw;
          }
        }
        SortEntry = SortEntry->pSortedNext;
        n++;
      }
      PlanePoly.Deallocate();
      delete PlaneTexture;
  }

  ShadowPoly.Deallocate();
  GradientShadowPoly.Deallocate();

  return iCount;
}

Export int RenderLines_Masked(Image *Image, FLine *Lines, Byte *Mask, Pixel Color, int Count, float XOffset, float YOffset) {
  if (!Image) return Failure;
  if (!Lines) return Failure;
  if (!Mask) return Failure;
  if (Count < 1) return Failure;
  FLine Line;
  Pixel CurrentColor;
  for (int l = 0; l < Count; l++) {
    if (Mask[l]) {
      CurrentColor = Color;
      CurrentColor[::Alpha] = AlphaLookup(Mask[l], CurrentColor[::Alpha]);
      Line = Lines[l];
      Line.Start.X -= XOffset;
      Line.Start.Y -= YOffset;
      Line.End.X -= XOffset;
      Line.End.Y -= YOffset;
      if (ClipFloatLine(Image, &Line)) {
        int x_distance = Line.End.X - Line.Start.X, y_distance = Line.End.Y - Line.Start.Y;
        int pixel_count = (_Max(abs(x_distance), abs(y_distance)));
        float x_increment = (x_distance / (float)pixel_count), y_increment = (y_distance / (float)pixel_count);
        float current_x = Line.Start.X, current_y = Line.Start.Y;
        for (int i = 0; i <= pixel_count; i++) {
          Image->setPixelAA(current_x, current_y, CurrentColor);
          current_x += x_increment;
          current_y += y_increment;
        }
      }
    }
  }
  return Success;
}

Export int SelectLines(Rectangle *Area, FLine *Lines, Byte *Mask, int Count, float XOffset, float YOffset) {
  if (!Lines) return Failure;
  if (!Mask) return Failure;
  if (Count < 1) return Failure;
  Rectangle RealArea;
  FLine Line;
  RealArea = *Area;
  RealArea.normalize();
  for (int l = 0; l < Count; l++) {
    Line = Lines[l];
    Line.Start.X -= XOffset;
    Line.Start.Y -= YOffset;
    Line.End.X -= XOffset;
    Line.End.Y -= YOffset;
    if (ClipFloatLine(&RealArea, &Line)) {
      Mask[l] = 255;
    } else {
      Mask[l] = 0;
    }
  }
  return Success;
}

Export int SelectPlanes(Rectangle *Area, Lighting::Plane *Planes, Byte *Mask, int Count, float XOffset, float YOffset) {
  if (!Planes) return Failure;
  if (!Mask) return Failure;
  if (Count < 1) return Failure;
  Rectangle RealArea;
  Rectangle PlaneArea;
  Lighting::Plane Plane;
  RealArea = *Area;
  RealArea.normalize();
  for (int p = 0; p < Count; p++) {
    Plane = Planes[p];
    Plane.Start.X -= XOffset;
    Plane.Start.Y -= YOffset;
    Plane.End.X -= XOffset;
    Plane.End.Y -= YOffset;
    PlaneArea.Left = _Min(Plane.Start.X, Plane.End.X);
    PlaneArea.Top = _Min(Plane.Start.Y, Plane.End.Y);
    PlaneArea.setRight(_Max(Plane.Start.X, Plane.End.X));
    PlaneArea.setBottom(_Max(Plane.Start.Y, Plane.End.Y));
    if (PlaneArea.intersect(RealArea)) {
      Mask[p] = 255;
    } else {
      Mask[p] = 0;
    }
  }
  return Success;
}

Export int RenderCollisionLines(Image *Image, FLine *Lines, int Count, float XOffset, float YOffset) {
  if (!Image) return Failure;
  if (!Lines) return Failure;
  if (Count < 1) return Failure;
  FLine Line;
  for (int l = 0; l < Count; l++) {
    Line = Lines[l];
    Line.Start.X -= XOffset;
    Line.Start.Y -= YOffset;
    Line.End.X -= XOffset;
    Line.End.Y -= YOffset;
    if (ClipFloatLine(Image, &Line)) {
      int x_distance = Line.End.X - Line.Start.X, y_distance = Line.End.Y - Line.Start.Y;
      int pixel_count = (_Max(abs(x_distance), abs(y_distance)));
      float x_increment = (x_distance / (float)pixel_count), y_increment = (y_distance / (float)pixel_count);
      float current_x = Line.Start.X, current_y = Line.Start.Y;
      for (int i = 0; i <= pixel_count; i++) {
        Image->setPixelAA(current_x, current_y, Pixel((Image->getPixelAA(current_x, current_y).V ^ 0xFFFFFF) | 0xFF000000));
        current_x += x_increment;
        current_y += y_increment;
      }
    }
  }
  return Success;
}

Export CollisionMatrix* CreateCollisionMatrix(int Width, int Height) {
  return new CollisionMatrix(Width, Height);
}

Export CollisionMatrix* CreateCollisionMatrixEx(int Width, int Height, int SectorWidth, int SectorHeight) {
  return new CollisionMatrix(Width, Height, SectorWidth, SectorHeight);
}

Export int AppendLinesToCollisionMatrix(CollisionMatrix *Matrix, FLine *Lines, int Count) {
  if (Matrix == Null) return Failure;
  if (Count < 1) return Trivial_Success;
  if (Lines == Null) return Failure;
  if (Matrix->addLines(Lines, Count)) {
    return Success;
  }
  return Failure;
}

Export int EraseCollisionMatrix(CollisionMatrix *Matrix) {
  if (Matrix == Null) return Failure;
  Matrix->erase();
  return Success;
}

Export int DeleteCollisionMatrix(CollisionMatrix *Matrix) {
  if (Matrix == Null) return Failure;
  delete Matrix;
  return Success;
}

Export Lighting::Matrix* CreateLightingMatrix(int Width, int Height) {
  return new Lighting::Matrix(Width, Height);
}

Export int AppendObstructionsToLightingMatrix(Lighting::Matrix *Matrix, Lighting::Obstruction *Obstructions, int Count) {
  if (Matrix == Null) return Failure;
  if (Count < 1) return Trivial_Success;
  if (Obstructions == Null) return Failure;
  if (Matrix->addObstructions(Obstructions, Count)) {
    return Success;
  }
  return Failure;
}

Export int AppendPlanesToLightingMatrix(Lighting::Matrix *Matrix, Lighting::Plane *Planes, int Count) {
  if (Matrix == Null) return Failure;
  if (Count < 1) return Trivial_Success;
  if (Planes == Null) return Failure;
  if (Matrix->addPlanes(Planes, Count)) {
    return Success;
  }
  return Failure;
}

Export int EraseLightingMatrix(Lighting::Matrix *Matrix) {
  if (Matrix == Null) return Failure;
  Matrix->erase();
  return Success;
}

Export int DeleteLightingMatrix(Lighting::Matrix *Matrix) {
  if (Matrix == Null) return Failure;
  delete Matrix;
  return Success;
}

typedef void SegmentCallback(Image* Buffer, int XPass, int YPass);

Export int RenderMap(Map *Map, MapCamera *Camera, SegmentCallback *FlipCallback, SegmentCallback *BackgroundCallback) {
  if (!Map) return Failure;
  if (!Camera) return Failure;
  if (!Camera->ScratchBuffer) return Failure;
  if (Map->LayerCount < 1) return Failure;
  if ((Map->Width < 1) || (Map->Height < 1)) return Failure;
  if (!(Map->Layers)) return Failure;
  if (!(Map->Layers[0].Tileset)) return Failure;

  MapLayer *Layer;
  DoubleWord TileWidth = Map->Layers[0].Tileset->TileWidth, TileHeight = Map->Layers[0].Tileset->TileHeight;
  DoubleWord RenderXStep = Camera->ScratchBuffer->Width, RenderYStep = Camera->ScratchBuffer->Height;
  DoubleWord XPasses = ceil(Camera->Area.Width / (float)RenderXStep / Camera->ScaleRatioX), YPasses = ceil(Camera->Area.Height / (float)RenderYStep / Camera->ScaleRatioY);

  bool Scale = (Camera->Scaler != Null) && ((Camera->ScaleRatioX != 1) || (Camera->ScaleRatioY != 1));
  int CameraX = Camera->ViewportX, CameraY = Camera->ViewportY;
  int CameraXOffset = 0, CameraYOffset = 0;
  DoubleWord CameraTileX = 0, CameraTileY = 0;
  int RenderX = 0, RenderY = 0;
  DoubleWord RenderWidth = ceil(Camera->ScratchBuffer->Width / (float)TileWidth), RenderHeight = ceil(Camera->ScratchBuffer->Height / (float)TileHeight);
  DoubleWord BlockTileX = 0, BlockTileY = 0;
  DoubleWord TileX = 0, TileY = 0;

  Rectangle rDest;
  Rectangle rOutput, rScratch;

  short *BlockPointer = Null, *RowPointer = Null, *TilePointer = Null;

  TileBlitter** TileBlitters = Null;
  TintedTileBlitter** TintBlitters = Null;

  TileBlitters = LookupAllocate< TileBlitter* >(Map->LayerCount);
  TintBlitters = LookupAllocate< TintedTileBlitter* >(Map->LayerCount);

  TileBlitter* pTileBlitter = Null;
  TintedTileBlitter* pTintBlitter = Null;

  for (DoubleWord Layers = 0; Layers < Map->LayerCount; ++Layers) {
    pTileBlitter = Null;
    pTintBlitter = Null;
    switch(Map->Layers[Layers].BlitMode) {
    default:
    case 0:
        pTileBlitter = BlitSimple_Normal_Opacity;
        pTintBlitter = BlitSimple_Normal_Tint_Opacity;
        break;
    case 1:
        pTileBlitter = BlitSimple_Automatic_Matte_Opacity;
        pTintBlitter = BlitSimple_Matte_Tint_Opacity;
        break;
    case 2:
        pTileBlitter = BlitSimple_Automatic_SourceAlpha_Opacity;
        pTintBlitter = BlitSimple_SourceAlpha_Tint_Opacity;
        break;
    case 3:
        pTileBlitter = BlitSimple_Additive_Opacity;
        break;
    case 4:
        pTileBlitter = BlitSimple_Subtractive_Opacity;
        break;
    case 6:
        pTileBlitter = BlitSimple_Screen_Opacity;
        break;
    case 7:
        pTileBlitter = BlitSimple_Multiply_Opacity;
        break;
    case 8:
        pTileBlitter = BlitSimple_Lightmap_Opacity;
        break;
    }
    TileBlitters[Layers] = pTileBlitter;
    TintBlitters[Layers] = pTintBlitter;
  }

  rScratch = Camera->ScratchBuffer->getRectangle();
  rOutput = rScratch;

  enableClipping = true;

  BlockPointer = Null;

  DoubleWord MaxTileX = Map->Width - 1;
  DoubleWord MaxTileY = Map->Height - 1;

  CameraXOffset = CameraX % TileWidth;
  CameraYOffset = CameraY % TileHeight;
  CameraTileX = CameraX / TileWidth;
  CameraTileY = CameraY / TileWidth;

  Image* iTile = Null;
  short Tile = 0;

  for (DoubleWord YPass = 0; YPass < YPasses; ++YPass) {
    BlockTileY = (YPass * RenderHeight) + CameraTileY;
    RenderY = ((BlockTileY - CameraTileY) * TileHeight) - CameraYOffset + Camera->Area.Top;
    for (DoubleWord XPass = 0; XPass < XPasses; ++XPass) {
      BlockTileX = (XPass * RenderWidth) + CameraTileX;
      RenderX = ((BlockTileX - CameraTileX) * TileWidth) - CameraXOffset + Camera->Area.Left;
      if (BackgroundCallback) {
        BackgroundCallback(Camera->ScratchBuffer, RenderX, RenderY);
      } else {
        FilterSimple_Fill(Camera->ScratchBuffer, Null, Camera->BackgroundColor);
        if (Camera->OutputBuffer) {
          if (Camera->BackgroundOpacity) {
            int sx = RenderX, sy = RenderY;
            if (RenderX < 0) {
              rDest.Left = -RenderX;
              rDest.Width = Camera->ScratchBuffer->Width + RenderX;
              sx = 0;
            } else {
              rDest.Left = 0;
              rDest.Width = Camera->ScratchBuffer->Width;
            }
            if (RenderY < 0) {
              rDest.Top = -RenderY;
              rDest.Height = Camera->ScratchBuffer->Height + RenderY;
              sy = 0;
            } else {
              rDest.Top = 0;
              rDest.Height = Camera->ScratchBuffer->Height;
            }
            if (Scale) {
              rOutput.Left = rDest.Left;
              rOutput.Top = rDest.Top;
              rOutput.Width = rDest.Width;
              rOutput.Height = rDest.Height;
              rScratch.Left = ceil(RenderX * Camera->ScaleRatioX);
              rScratch.Top = ceil(RenderY * Camera->ScaleRatioY);
              rScratch.Width = ceil(rOutput.Width * Camera->ScaleRatioX);
              rScratch.Height = ceil(rOutput.Height * Camera->ScaleRatioY);
              BlitResample_Normal_Opacity(Camera->ScratchBuffer, Camera->OutputBuffer, &rOutput, &rScratch, Camera->Scaler, Camera->BackgroundOpacity);
            } else {
              BlitSimple_Normal_Opacity(Camera->ScratchBuffer, Camera->OutputBuffer, &rDest, sx, sy, Camera->BackgroundOpacity);
            }
          }
        }
      }

      enableClipping = false;
      rDest.Left = 0;
      rDest.Top = 0;
      rDest.Width = TileWidth;
      rDest.Height = TileHeight;
      for (DoubleWord Layers = 0; Layers < Map->LayerCount; ++Layers) {
        Layer = &(Map->Layers[Layers]);
        iTile = Null;
        Tile = Layer->IgnoredTile;
        pTileBlitter = TileBlitters[Layers];
        pTintBlitter = TintBlitters[Layers];

        rDest.Top = 0;

        for (TileY = 0; TileY < RenderHeight; ++TileY) {
          if (Layer->WrapY) {
              RowPointer = Layer->Tiles + (Map->Width * ((TileY + BlockTileY) % MaxTileY));
          } else {
              RowPointer = Layer->Tiles + (Map->Width * (TileY + BlockTileY));
          }
          TilePointer = RowPointer + BlockTileX;

          rDest.Left = 0;

          for (TileX = 0; TileX < RenderWidth; ++TileX) {
            if (Layer->WrapX) {
                TilePointer = RowPointer + ((TileX + BlockTileX) % MaxTileX);
            }

            if (*TilePointer != Tile) {
              Tile = *TilePointer;
              iTile = Layer->Tileset->tile(Tile, Layer->AnimationTable);
            }

            if (Tile) {
              pTileBlitter(Camera->ScratchBuffer, iTile, &rDest, 0, 0, Layer->Opacity);
            }

            ++TilePointer; // Isn't necessary if WrapX is true, but I suspect an If here would just hurt performance anyway. Increments are fast.
            rDest.Left += TileWidth;
          }

          rDest.Top += TileHeight;

        }

      }
      enableClipping = true;

      if (FlipCallback) {
        FlipCallback(Camera->ScratchBuffer, RenderX, RenderY);
      } else {
        if (Camera->OutputBuffer) {
          rOutput.Left = ceil(RenderX * Camera->ScaleRatioX);
          rOutput.Top = ceil(RenderY * Camera->ScaleRatioY);
          rOutput.Width = ceil(Camera->ScratchBuffer->Width * Camera->ScaleRatioX);
          rOutput.Height = ceil(Camera->ScratchBuffer->Height * Camera->ScaleRatioY);
          rScratch.Left = 0;
          rScratch.Top = 0;
          rScratch.Width = Camera->ScratchBuffer->Width;
          rScratch.Height = Camera->ScratchBuffer->Height;
          if (Scale) {
            BlitResample_Normal_Opacity(Camera->OutputBuffer, Camera->ScratchBuffer, &rOutput, &rScratch, Camera->Scaler, Camera->MapOpacity);
          } else {
            BlitSimple_Normal_Opacity(Camera->OutputBuffer, Camera->ScratchBuffer, &rOutput, 0, 0, Camera->MapOpacity);
          }
        }
      }
    }
  }

  LookupDeallocate< TileBlitter* >(TileBlitters);
  LookupDeallocate< TintedTileBlitter* >(TintBlitters);

  return Success;
}

//*/

void CollisionMatrix::resize(int W, int H) {
  this->deallocate();
  this->Width = W;
  this->Height = H;
  this->Sectors.resize(W * H);
  for (DoubleWord i = 0; i < this->Sectors.size(); i++) {
    this->Sectors[i] = new CollisionSector(this->SectorWidth, this->SectorHeight);
  }
}

bool CollisionMatrix::addLines(FLine *Lines, int Count) {
  if (Count < 1) return false;
  if (Lines == Null) return false;
  for (int y = 0; y < this->Height; y++) {
    for (int x = 0; x < this->Width; x++) {
      if (!this->getSector(x, y)->addLines(Lines, Count, x * this->SectorWidth, y * this->SectorHeight)) {
        return false;
      }
    }
  }
  return true;
}

bool CollisionMatrix::addSprite(SpriteParam *Sprite) {
  FRect rect = Sprite->getRect();
  int x1 = ClipValue(floor(rect.X1 / this->SectorWidth)-1, 0, this->Width - 1);
  int y1 = ClipValue(floor(rect.Y1 / this->SectorHeight)-1, 0, this->Height - 1);
  int x2 = ClipValue(ceil(rect.X2 / this->SectorWidth)+1, 0, this->Width - 1);
  int y2 = ClipValue(ceil(rect.Y2 / this->SectorHeight)+1, 0, this->Height - 1);
  bool added = false;
  for (int y = y1; y <= y2; y++) {
    for (int x = x1; x <= x2; x++) {
      added |= this->getSector(x, y)->addSprite(Sprite, x * this->SectorWidth, y * this->SectorHeight);
    }
  }
  if (!added)
    Stragglers.push_back(Sprite);
  return true;
}

bool CollisionMatrix::clearSprites() {
  for (int y = 0; y < this->Height; y++) {
    for (int x = 0; x < this->Width; x++) {
      this->getSector(x, y)->clearSprites();
    }
  }
  Stragglers.clear();
  return true;
}

bool CollisionMatrix::collisionCheck(FRect *Rectangle) const {
  int xMin = Rectangle->X1 / this->SectorWidth, yMin = Rectangle->Y1 / this->SectorHeight, 
      xMax = Rectangle->X2 / this->SectorWidth, yMax = Rectangle->Y2 / this->SectorHeight;
  xMin = ClipValue(xMin, this->Width - 1);
  yMin = ClipValue(yMin, this->Height - 1);
  xMax = ClipValue(xMax, this->Width - 1);
  yMax = ClipValue(yMax, this->Height - 1);
  for (int y = yMin; y <= yMax; y++) {
    for (int x = xMin; x <= xMax; x++) {
      if (this->getSector(x, y)->collisionCheck(Rectangle, x * this->SectorWidth, y * this->SectorHeight)) {
        return true;
      }
    }
  }
  return false;
}

bool CollisionMatrix::collisionCheck(FRect *Rectangle, SimplePolygon *Polygon) const {
  int xMin = Rectangle->X1 / this->SectorWidth, yMin = Rectangle->Y1 / this->SectorHeight, 
      xMax = Rectangle->X2 / this->SectorWidth, yMax = Rectangle->Y2 / this->SectorHeight;
  xMin = ClipValue(xMin, this->Width - 1);
  yMin = ClipValue(yMin, this->Height - 1);
  xMax = ClipValue(xMax, this->Width - 1);
  yMax = ClipValue(yMax, this->Height - 1);
  for (int y = yMin; y <= yMax; y++) {
    for (int x = xMin; x <= xMax; x++) {
      if (this->getSector(x, y)->collisionCheck(Rectangle, Polygon, x * this->SectorWidth, y * this->SectorHeight)) {
        return true;
      }
    }
  }
  return false;
}

void CollisionMatrix::erase() {
  for (int y = 0; y < this->Height; y++) {
    for (int x = 0; x < this->Width; x++) {
	  this->getSector(x, y)->Lines.clear();
    }
  }
}

bool CollisionSector::collisionCheck(FRect *Rectangle, int XOffset, int YOffset) {
  if (CheckLineCollide(Rectangle, this->Lines)) {
    return true;
  }
  return false;
}

bool CollisionSector::collisionCheck(FRect *Rectangle, SimplePolygon *Polygon, int XOffset, int YOffset) {
  if (CheckLineCollide2(Rectangle, Polygon, this->Lines)) {
    return true;
  }
  return false;
}

bool CollisionSector::addLines(FLine *Lines, int Count, int XOffset, int YOffset) {
  if (Count < 1) return false;
  if (Lines == Null) return false;
  FLine *Pointer = Lines, CurrentLine;
  Rectangle ThisRectangle;
  ThisRectangle.setValues(XOffset - 1, YOffset - 1, this->Width + 2, this->Height + 2);
  for (int i = 0; i < Count; i++) {
    CurrentLine = *Pointer;
    if (ClipFloatLine(&ThisRectangle, &CurrentLine)) {
      this->Lines.push_back(CurrentLine);
    }
    Pointer++;
  }
  return true;
}

bool CollisionSector::addSprite(SpriteParam *Sprite, int XOffset, int YOffset) {
  FRect ThisRectangle(XOffset - 1, YOffset - 1, this->Width + 2, this->Height + 2);
  if (Sprite->touches(&ThisRectangle)) {
    this->Sprites.push_back(Sprite);
    return true;
  }
  return false;
}

bool CollisionSector::removeSprite(SpriteParam *Sprite) {
  std::vector<SpriteParam*>::iterator iter = std::find(this->Sprites.begin(), this->Sprites.end(), Sprite);
  if (iter != this->Sprites.end()) {
    this->Sprites.erase(iter);
    return true;
  }
  return false;
}

bool CollisionSector::clearSprites() {
  this->Sprites.clear();
  return true;
}

void Lighting::Matrix::resize(int W, int H) {
  this->deallocate();
  this->Width = W;
  this->Height = H;
  this->Sectors.resize(W * H);
  for (DoubleWord i = 0; i < this->Sectors.size(); i++) {
    this->Sectors[i] = new Lighting::Sector(this->SectorWidth, this->SectorHeight);
  }
}

void Lighting::Matrix::erase() {
  for (int y = 0; y < this->Height; y++) {
    for (int x = 0; x < this->Width; x++) {
	  this->getSector(x, y)->Obstructions.clear();
	  this->getSector(x, y)->Planes.clear();
    }
  }
}

bool Lighting::Matrix::addObstructions(Lighting::Obstruction *Obstructions, int Count) {
  if (Count < 1) return false;
  if (Obstructions == Null) return false;
  for (int y = 0; y < this->Height; y++) {
    for (int x = 0; x < this->Width; x++) {
      if (!this->getSector(x, y)->addObstructions(Obstructions, Count, x * this->SectorWidth, y * this->SectorHeight)) {
        return false;
      }
    }
  }
  return true;
}

bool Lighting::Sector::addObstructions(Lighting::Obstruction *Obstructions, int Count, int XOffset, int YOffset) {
  if (Count < 1) return false;
  if (Obstructions == Null) return false;
  Lighting::Obstruction *Pointer = Obstructions, CurrentObstruction;
  Rectangle ThisRectangle;
  ThisRectangle.setValues(XOffset, YOffset, this->Width, this->Height);
  for (int i = 0; i < Count; i++) {
    CurrentObstruction = *Pointer;
    if (ClipFloatLine(&ThisRectangle, (FLine*)&CurrentObstruction)) {
      this->Obstructions.push_back(*Pointer);
    }
    Pointer++;
  }
  return true;
}

bool Lighting::Matrix::addPlanes(Lighting::Plane *Planes, int Count) {
  if (Count < 1) return false;
  if (Planes == Null) return false;
  for (int y = 0; y < this->Height; y++) {
    for (int x = 0; x < this->Width; x++) {
      if (!this->getSector(x, y)->addPlanes(Planes, Count, x * this->SectorWidth, y * this->SectorHeight)) {
        return false;
      }
    }
  }
  return true;
}

bool Lighting::Sector::addPlanes(Lighting::Plane *Planes, int Count, int XOffset, int YOffset) {
  if (Count < 1) return false;
  if (Planes == Null) return false;
  Lighting::Plane *Pointer = Planes, CurrentPlane;
  Rectangle ThisRectangle;
  ThisRectangle.setValues(XOffset, YOffset, this->Width, this->Height);
  for (int i = 0; i < Count; i++) {
    CurrentPlane = *Pointer;
/*
	if (ClipFloatLine(&ThisRectangle, (FLine*)&CurrentPlane)) {
      this->Planes.push_back(CurrentPlane);
    }
*/
    Pointer++;
  }
  return true;
}

Export int IterateSprites(SpriteParam *sprites, SpriteIterator *func) {
  // NYI
  return 0;
}