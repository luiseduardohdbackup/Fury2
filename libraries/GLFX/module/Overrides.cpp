#include "../header/GLFX.hpp"
#include "../header/Tilemap.hpp"
#include "../header/WindowSkin.hpp"

using namespace SoftFX;
using namespace GL;
using namespace BlendModes;
using namespace ScaleModes;

#define contextCheck(param) \
  if (!checkNamedTag(param, Context)) { return Failure; }
#define lockCheck(param) \
  if (!GetImageLocked(param)) { return Failure; }
#define clipCheck(img, rect) \
  if (!ClipRectangle_ImageClipRect(&rect, img)) return Trivial_Success;

void setScaler(int Scaler) {
	if (Scaler == GetBilinearScaler()) {
		setScaleMode<Bilinear>();
	} else if (Scaler == GetBilinearWrapScaler()) {
		setScaleMode<Bilinear_Wrap>();
	} else if (Scaler == GetBilinearClampScaler()) {
		setScaleMode<Bilinear_Clamp>();
	} else if (Scaler == GetLinearWrapScaler()) {
		setScaleMode<Linear_Wrap>();
	} else if (Scaler == GetLinearClampScaler()) {
		setScaleMode<Linear_Clamp>();
	} else {
		setScaleMode<Linear>();
	}
}

void setRenderer(int Renderer, Pixel RenderArgument) {
  if ((Renderer == GetSourceAlphaRenderer()) || (Renderer == GetMergeRenderer())) {
    setBlendMode<SourceAlpha>();
    if (RenderArgument[::Alpha] > 0) {
      setFogColor(RenderArgument);
      enableFog();
      setFogOpacity(RenderArgument[::Alpha] / 255.0f);
    } else {
      disableFog();
    }
  } else if (Renderer == GetFontSourceAlphaRenderer()) {
    setBlendMode<Font_SourceAlpha>();
    setVertexColor(RenderArgument);
    disableFog();
  } else if (Renderer == GetAdditiveRenderer()) {
    setBlendMode<Additive>();
    disableFog();
  } else if (Renderer == GetSubtractiveRenderer()) { 
    setBlendMode<Subtractive>();
    disableFog();
  } else if (Renderer == GetAdditiveSourceAlphaRenderer()) {
    setBlendMode<Additive_SourceAlpha>();
    disableFog();
  } else if (Renderer == GetSubtractiveSourceAlphaRenderer()) { 
    setBlendMode<Subtractive_SourceAlpha>();
    disableFog();
  } else {
    setBlendMode<Normal>();
    if (RenderArgument[::Alpha] > 0) {
      setFogColor(RenderArgument);
      enableFog();
      setFogOpacity(RenderArgument[::Alpha] / 255.0f);
    } else {
      disableFog();
    }
  }
  setBlendColor(White);
}

void setMaskRenderer(int Renderer, Pixel RenderArgument) {
  if ((Renderer == GetSourceAlphaRenderer()) || (Renderer == GetMergeRenderer())) {
    setBlendMode<Mask_SourceAlpha>();
    if (RenderArgument[::Alpha] > 0) {
      setFogColor(RenderArgument);
      enableFog();
      setFogOpacity(RenderArgument[::Alpha] / 255.0f);
    } else {
      disableFog();
    }
  } else {
    setBlendMode<Mask_Normal>();
    if (RenderArgument[::Alpha] > 0) {
      setFogColor(RenderArgument);
      enableFog();
      setFogOpacity(RenderArgument[::Alpha] / 255.0f);
    } else {
      disableFog();
    }
  }
}

defOverride(Allocate) {
	readParam(int, Image, 0);
	readParam(int, Width, 1);
	readParam(int, Height, 2);
  if (Width < 1) return 0;
  if (Height < 1) return 0;
  if (Image == Null) return 0;
  flushImageHeap();
  Global->Framebuffer = Image;
  PIXELFORMATDESCRIPTOR pfdFormat = { 
      sizeof(PIXELFORMATDESCRIPTOR),   // size of this pfd 
      1,                     // version number 
      PFD_DRAW_TO_WINDOW |   // support window 
      PFD_SUPPORT_OPENGL |   // support OpenGL 
      PFD_DOUBLEBUFFER,      // double buffered 
      PFD_TYPE_RGBA,         // RGBA type 
      32,                    // 32-bit color depth 
      0, 0, 0, 0, 0, 0,      // color bits ignored 
      0,                     // no alpha buffer 
      0,                     // shift bit ignored 
      0,                     // no accumulation buffer 
      0, 0, 0, 0,            // accum bits ignored 
      0,                     // no z-buffer 
      0,                     // no stencil buffer 
      0,                     // no auxiliary buffer 
      PFD_MAIN_PLANE,        // main layer 
      0,                     // reserved 
      0, 0, 0                // layer masks ignored 
  }; 
  int iFormat = 0;
  iFormat = ChoosePixelFormat(Global->DC, &pfdFormat);
  SetPixelFormat(Global->DC, iFormat, &pfdFormat);
  Global->Context = wglCreateContext(Global->DC);
  wglMakeCurrent(Global->DC, Global->Context);
  glewInit();
  GL::init();
  glEnable(GL_SCISSOR_TEST);
  glShadeModel(GL_SMOOTH);
  glEnable(GL_BLEND);
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  glOrtho(0, Global->OutputWidth, Global->OutputHeight, 0, -1, 1);
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();
  glViewport(0, 0, Global->OutputWidth, Global->OutputHeight);
  glScissor(0, 0, Global->OutputWidth, Global->OutputHeight);
  glLineWidth(1.0f);
  setNamedTagAndKey(Image, Context, Global->Context);
  setNamedTag(Image, DC, Global->DC);
  SetImageWidth(Image, Width);
  SetImageHeight(Image, Height);
  SetImagePitch(Image, 0);
  SetImagePointer(Image, Null);
  SetImageLocked(Image, true);
	return 0;
}

defOverride(Clear) {
  readParam(int, Image, 0);
  contextCheck(Image);
  lockCheck(Image);
  selectContext(Image);
  endDraw();
  glClearColor(0, 0, 0, 0);
  glClear(GL_COLOR_BUFFER_BIT);
  SetImageDirty(Image, 1);
  return Success;
}

defOverride(Lock) {
  readParam(int, Image, 0);
  contextCheck(Image);
  if (GetImageLocked(Image)) return Failure;
  if (GetImageDirty(Image)) {
    copyImageToFramebuffer(Image);
  }
  SetImageLocked(Image, 1);
  SetImageDirty(Image, 0);
  return Success;
}

defOverride(Unlock) {
  readParam(int, Image, 0);
  contextCheck(Image);
  if (!GetImageLocked(Image)) return Failure;
  SetImageLocked(Image, 0);
  if (GetImageDirty(Image)) {
    copyFramebufferToImage(Image);
  }
  SetImageDirty(Image, 0);
  return Success;
}

defOverride(FilterSimple_Fill) {
  readParam(int, Image, 0);
  contextCheck(Image);
  lockCheck(Image);
  readParamRect(Area, 1, Image);
  readParam(Pixel, Color, 2);
  selectContext(Image);
  clipCheck(Image, Area);
  disableTextures();
  setBlendMode<Normal>();
  setVertexColor(Color);
  setBlendColor(White);
  drawRectangle(Area);
  SetImageDirty(Image, 1);
  return Success;
}

defOverride(FilterSimple_Fill_Opacity) {
  readParam(int, Image, 0);
  contextCheck(Image);
  lockCheck(Image);
  readParamRect(Area, 1, Image);
  readParam(Pixel, Color, 2);
  readParam(int, Opacity, 3);
  selectContext(Image);
  clipCheck(Image, Area);
  disableTextures();
  setBlendMode<Normal>();
  setVertexColor(Color);
  setBlendColor(Pixel(255,255,255,Opacity));
  drawRectangle(Area);
  SetImageDirty(Image, 1);
  return Success;
}

defOverride(FilterSimple_Fill_SourceAlpha) {
  readParam(int, Image, 0);
  contextCheck(Image);
  lockCheck(Image);
  readParamRect(Area, 1, Image);
  readParam(Pixel, Color, 2);
  selectContext(Image);
  clipCheck(Image, Area);
  disableTextures();
  setBlendMode<SourceAlpha>();
  setVertexColor(Color);
  setBlendColor(White);
  drawRectangle(Area);
  SetImageDirty(Image, 1);
  return Success;
}

defOverride(FilterSimple_Fill_SourceAlpha_Opacity) {
  readParam(int, Image, 0);
  contextCheck(Image);
  lockCheck(Image);
  readParamRect(Area, 1, Image);
  readParam(Pixel, Color, 2);
  readParam(int, Opacity, 3);
  selectContext(Image);
  clipCheck(Image, Area);
  disableTextures();
  setBlendMode<SourceAlpha>();
  setVertexColor(MultiplyAlpha(Color, Opacity));
  setBlendColor(White);
  drawRectangle(Area);
  SetImageDirty(Image, 1);
  return Success;
}

defOverride(FilterSimple_Fill_Additive) {
  readParam(int, Image, 0);
  contextCheck(Image);
  lockCheck(Image);
  readParamRect(Area, 1, Image);
  readParam(Pixel, Color, 2);
  selectContext(Image);
  clipCheck(Image, Area);
  disableTextures();
  setBlendMode<Additive>();
  setVertexColor(Color);
  setBlendColor(White);
  drawRectangle(Area);
  SetImageDirty(Image, 1);
  return Success;
}

defOverride(FilterSimple_Fill_Additive_Opacity) {
  readParam(int, Image, 0);
  contextCheck(Image);
  lockCheck(Image);
  readParamRect(Area, 1, Image);
  readParam(Pixel, Color, 2);
  readParam(int, Opacity, 3);
  selectContext(Image);
  clipCheck(Image, Area);
  disableTextures();
  setBlendMode<Additive>();
  setVertexColor(Color);
  setBlendColor(Pixel(255, 255, 255, Opacity));
  drawRectangle(Area);
  SetImageDirty(Image, 1);
  return Success;
}

defOverride(FilterSimple_Fill_Subtractive) {
  readParam(int, Image, 0);
  contextCheck(Image);
  lockCheck(Image);
  readParamRect(Area, 1, Image);
  readParam(Pixel, Color, 2);
  selectContext(Image);
  clipCheck(Image, Area);
  disableTextures();
  setBlendMode<Subtractive>();
  setVertexColor(Color);
  setBlendColor(White);
  drawRectangle(Area);
  SetImageDirty(Image, 1);
  return Success;
}

defOverride(FilterSimple_Fill_Subtractive_Opacity) {
  readParam(int, Image, 0);
  contextCheck(Image);
  lockCheck(Image);
  readParamRect(Area, 1, Image);
  readParam(Pixel, Color, 2);
  readParam(int, Opacity, 3);
  selectContext(Image);
  clipCheck(Image, Area);
  disableTextures();
  setBlendMode<Subtractive>();
  setVertexColor(Color);
  setBlendColor(Pixel(255, 255, 255, Opacity));
  drawRectangle(Area);
  SetImageDirty(Image, 1);
  return Success;
}

void BlitSimple_Core(Override::OverrideParameters *Parameters) {
  readParam(int, Dest, 0);
  readParam(int, Source, 1);
  readParamRect(Area, 2, Null);
  readParam(int, SourceX, 3);
  readParam(int, SourceY, 4);
  FX::Rectangle AreaCopy = Area;
  if (Clip2D_SimpleRect(&Area, Dest, Source, &AreaCopy, SourceX, SourceY)) {
    enableTextures();
    selectImageAsTexture(Source);
    setScaleMode<Linear>();
    Texture* tex = getTexture(Source);
    if ((Area.Width == tex->Width) && (Area.Height == tex->Height) && (SourceX == 0) && (SourceY == 0)) { 
      drawTexturedRectangle(Area, tex->U1, tex->V1, tex->U2, tex->V2);
    } else {
      float U1 = tex->U(SourceX), V1 = tex->V(SourceY);
      float U2 = tex->U(SourceX + Area.Width), V2 = tex->V(SourceY + Area.Height);
      drawTexturedRectangle(Area, U1, V1, U2, V2);
    }
    SetImageDirty(Dest, 1);
  }
}

void BlitSimple_FBTex_Core(Override::OverrideParameters *Parameters) {
  readParam(int, Dest, 0);
  readParam(int, Source, 1);
  readParamRect(Area, 2, Null);
  readParam(int, SourceX, 3);
  readParam(int, SourceY, 4);
  FX::Rectangle AreaCopy = Area;
  if (Clip2D_SimpleRect(&Area, Dest, Source, &AreaCopy, SourceX, SourceY)) {
    selectImageAsTexture(Dest);
    copyFramebufferToTexture(getTexture(Dest), Dest);
    int texWidth = powerOfTwo(GetImageWidth(Source));
    int texHeight = powerOfTwo(GetImageHeight(Source));
    float xScale = 1.0f / texWidth;
    float yScale = 1.0f / texHeight;
    float U1 = (SourceX) * xScale, V1 = (SourceY) * yScale;
    float U2 = U1 + ((Area.Width) * xScale), V2 = V1 + ((Area.Height) * yScale);
    texWidth = powerOfTwo(GetImageWidth(Dest));
    texHeight = powerOfTwo(GetImageHeight(Dest));
    xScale = 1.0f / texWidth;
    yScale = 1.0f / texHeight;
    float U3 = (Area.Left) * xScale, V3 = (Area.Top) * yScale;
    float U4 = U3 + (Area.Width * xScale), V4 = V3 + (Area.Height * yScale);
    enableTexture<0>();
    enableTexture<1>();
    selectImageAsTextureN<0>(Source);
    selectImageAsTextureN<1>(Dest);
    setScaleMode<Linear>();
    draw2TexturedRectangle(Area, U1, V1, U2, V2, U3, V3, U4, V4);
    SetImageDirty(Dest, 1);
  }
}

defOverride(BlitSimple_Normal) {
  readParam(int, Dest, 0);
  contextCheck(Dest);
  lockCheck(Dest);
  selectContext(Dest);
  setBlendMode<Normal>();
  setTextureColor(White);
  setVertexColor(White);
  setBlendColor(White);
  BlitSimple_Core(Parameters);
  return Success;
}

defOverride(BlitSimple_Normal_Opacity) {
  readParam(int, Dest, 0);
  readParam(int, Opacity, 5);
  contextCheck(Dest);
  lockCheck(Dest);
  selectContext(Dest);
  setBlendMode<Normal>();
  setVertexColor(White);
  setTextureColor(White);
  setBlendColor(Pixel(255, 255, 255, Opacity));
  BlitSimple_Core(Parameters);
  return Success;
}

defOverride(BlitSimple_Subtractive) {
  readParam(int, Dest, 0);
  contextCheck(Dest);
  lockCheck(Dest);
  selectContext(Dest);
  setBlendMode<Subtractive>();
  setTextureColor(White);
  setVertexColor(White);
  setBlendColor(White);
  BlitSimple_Core(Parameters);
  return Success;
}

defOverride(BlitSimple_Subtractive_Opacity) {
  readParam(int, Dest, 0);
  readParam(int, Opacity, 5);
  contextCheck(Dest);
  lockCheck(Dest);
  selectContext(Dest);
  setBlendMode<Subtractive>();
  setTextureColor(White);
  setVertexColor(White);
  setBlendColor(Pixel(255, 255, 255, Opacity));
  BlitSimple_Core(Parameters);
  return Success;
}

defOverride(BlitSimple_Additive) {
  readParam(int, Dest, 0);
  contextCheck(Dest);
  lockCheck(Dest);
  selectContext(Dest);
  setBlendMode<Additive>();
  setTextureColor(White);
  setVertexColor(White);
  setBlendColor(White);
  BlitSimple_Core(Parameters);
  return Success;
}

defOverride(BlitSimple_Additive_Opacity) {
  readParam(int, Dest, 0);
  readParam(int, Opacity, 5);
  contextCheck(Dest);
  lockCheck(Dest);
  selectContext(Dest);
  setBlendMode<Additive>();
  setTextureColor(White);
  setVertexColor(White);
  setBlendColor(Pixel(255, 255, 255, Opacity));
  BlitSimple_Core(Parameters);
  return Success;
}

defOverride(BlitSimple_Subtractive_SourceAlpha) {
  readParam(int, Dest, 0);
  contextCheck(Dest);
  lockCheck(Dest);
  selectContext(Dest);
  setBlendMode<Subtractive_SourceAlpha>();
  setTextureColor(White);
  setVertexColor(White);
  BlitSimple_Core(Parameters);
  return Success;
}

defOverride(BlitSimple_Subtractive_SourceAlpha_Opacity) {
  readParam(int, Dest, 0);
  readParam(int, Opacity, 5);
  contextCheck(Dest);
  lockCheck(Dest);
  selectContext(Dest);
  setBlendMode<Subtractive_SourceAlpha>();
  setTextureColor(White);
  setVertexColor(Pixel(255, 255, 255, Opacity));
  BlitSimple_Core(Parameters);
  return Success;
}

defOverride(BlitSimple_Additive_SourceAlpha) {
  readParam(int, Dest, 0);
  contextCheck(Dest);
  lockCheck(Dest);
  selectContext(Dest);
  setBlendMode<Additive_SourceAlpha>();
  setTextureColor(White);
  setVertexColor(White);
  BlitSimple_Core(Parameters);
  return Success;
}

defOverride(BlitSimple_Additive_SourceAlpha_Opacity) {
  readParam(int, Dest, 0);
  readParam(int, Opacity, 5);
  contextCheck(Dest);
  lockCheck(Dest);
  selectContext(Dest);
  setBlendMode<Additive_SourceAlpha>();
  setTextureColor(White);
  setVertexColor(Pixel(255, 255, 255, Opacity));
  BlitSimple_Core(Parameters);
  return Success;
}

defOverride(BlitSimple_Multiply) {
  readParam(int, Dest, 0);
  contextCheck(Dest);
  lockCheck(Dest);
  selectContext(Dest);
  enableTextures();
  setBlendMode<Multiply>();
  setTextureColor(White);
  setVertexColor(White);
  BlitSimple_Core(Parameters);
  return Success;
}

defOverride(BlitSimple_Multiply_Opacity) {
  readParam(int, Dest, 0);
  readParam(int, Opacity, 5);
  contextCheck(Dest);
  lockCheck(Dest);
  selectContext(Dest);
  enableTextures();
  setBlendMode<Multiply>();
  setVertexColor(White);
  setTextureColor(Pixel(Opacity, Opacity, Opacity, Opacity));
  BlitSimple_Core(Parameters);
  return Success;
}

defOverride(BlitSimple_Lightmap) {
  readParam(int, Dest, 0);
  contextCheck(Dest);
  lockCheck(Dest);
  selectContext(Dest);
  enableTexture<0>();
  setTextureColor(White);
  enableTexture<1>();
  setBlendMode<Lightmap>();
  setVertexColor(White);
  setBlendColor(White);
  BlitSimple_FBTex_Core(Parameters);
  disableTexture<1>();
  switchTextureStage<0>();
  return Success;
}

defOverride(BlitSimple_Lightmap_Opacity) {
  readParam(int, Dest, 0);
  readParam(int, Opacity, 5);
  contextCheck(Dest);
  lockCheck(Dest);
  selectContext(Dest);
  enableTexture<0>();
  setTextureColor(White);
  enableTexture<1>();
  setBlendMode<Lightmap>();
  setVertexColor(White);
  setBlendColor(Pixel(Opacity, Opacity, Opacity, Opacity));
  BlitSimple_FBTex_Core(Parameters);
  disableTexture<1>();
  switchTextureStage<0>();
  return Success;
}

passOverride(BlitSimple_Lightmap_RGB, BlitSimple_Lightmap);

passOverride(BlitSimple_Lightmap_RGB_Opacity, BlitSimple_Lightmap_Opacity);

defOverride(BlitSimple_Normal_Tint) {
  readParam(int, Dest, 0);
  readParam(Pixel, Color, 5);
  contextCheck(Dest);
  lockCheck(Dest);
  selectContext(Dest);
  setBlendMode<Normal>();
  setVertexColor(White);
  setBlendColor(White);
  setFogColor(Color);
  setFogOpacity(Color[::Alpha] / 255.0f);
  enableFog();
  BlitSimple_Core(Parameters);
  disableFog();
  return Success;
}

defOverride(BlitSimple_Normal_Tint_Opacity) {
  readParam(int, Dest, 0);
  readParam(Pixel, Color, 5);
  readParam(int, Opacity, 6);
  contextCheck(Dest);
  lockCheck(Dest);
  selectContext(Dest);
  setBlendMode<Normal>();
  setVertexColor(White);
  setBlendColor(Pixel(255, 255, 255, Opacity));
  setFogColor(Color);
  setFogOpacity(Color[::Alpha] / 255.0f);
  enableFog();
  BlitSimple_Core(Parameters);
  disableFog();
  return Success;
}

void prepMatte(int Image) {
  Texture* tex = getTexture(Image);
  if (tex == 0) selectImageAsTexture(Image);
  tex = getTexture(Image);
  if (tex) {
    if (tex->MatteOptimized) {
    } else {
      Pixel matteColor = GetImageMatteColor(Image);
      FilterSimple_Replace(Image, 0, matteColor, Pixel(0, 0, 0, 0));
      SetImageMatteColor(Image, Pixel(0, 0, 0, 0));
      uploadImageToTexture(tex, Image);
      tex->MatteOptimized = true;
    }
  }
}

defOverride(BlitSimple_Matte) {
  readParam(int, Dest, 0);
  readParam(int, Source, 1);
  contextCheck(Dest);
  lockCheck(Dest);
  selectContext(Dest);
  setBlendMode<SourceAlpha>();
  setVertexColor(White);
  prepMatte(Source);
  BlitSimple_Core(Parameters);
  return Success;
}

defOverride(BlitSimple_Matte_Opacity) {
  readParam(int, Dest, 0);
  readParam(int, Source, 1);
  readParam(int, Opacity, 5);
  contextCheck(Dest);
  lockCheck(Dest);
  selectContext(Dest);
  setBlendMode<SourceAlpha>();
  setVertexColor(Pixel(255, 255, 255, Opacity));
  prepMatte(Source);
  BlitSimple_Core(Parameters);
  return Success;
}

defOverride(BlitSimple_Matte_Tint) {
  readParam(int, Dest, 0);
  readParam(int, Source, 1);
  readParam(Pixel, Color, 5);
  contextCheck(Dest);
  lockCheck(Dest);
  selectContext(Dest);
  setBlendMode<SourceAlpha>();
  setVertexColor(White);
  setFogColor(Color);
  setFogOpacity(Color[::Alpha] / 255.0f);
  enableFog();
  prepMatte(Source);
  BlitSimple_Core(Parameters);
  disableFog();
  return Success;
}

defOverride(BlitSimple_Matte_Tint_Opacity) {
  readParam(int, Dest, 0);
  readParam(int, Source, 1);
  readParam(Pixel, Color, 5);
  readParam(int, Opacity, 6);
  contextCheck(Dest);
  lockCheck(Dest);
  selectContext(Dest);
  setBlendMode<SourceAlpha>();
  setVertexColor(Pixel(255, 255, 255, Opacity));
  setFogColor(Color);
  setFogOpacity(Color[::Alpha] / 255.0f);
  enableFog();
  prepMatte(Source);
  BlitSimple_Core(Parameters);
  disableFog();
  return Success;
}

defOverride(BlitSimple_SourceAlpha) {
  readParam(int, Dest, 0);
  contextCheck(Dest);
  lockCheck(Dest);
  selectContext(Dest);
  setBlendMode<SourceAlpha>();
  setTextureColor(White);
  setVertexColor(White);
  BlitSimple_Core(Parameters);
  return Success;
}

defOverride(BlitSimple_SourceAlpha_Opacity) {
  readParam(int, Dest, 0);
  readParam(int, Opacity, 5);
  contextCheck(Dest);
  lockCheck(Dest);
  selectContext(Dest);
  setBlendMode<SourceAlpha>();
  setTextureColor(White);
  setVertexColor(Pixel(255, 255, 255, Opacity));
  BlitSimple_Core(Parameters);
  return Success;
}

passOverride(BlitSimple_SourceAlphaMatte, BlitSimple_SourceAlpha)

passOverride(BlitSimple_SourceAlphaMatte_Opacity, BlitSimple_SourceAlpha_Opacity)

passOverride(BlitSimple_Merge, BlitSimple_SourceAlpha)

passOverride(BlitSimple_Merge_Opacity, BlitSimple_SourceAlpha_Opacity)

defOverride(BlitSimple_SourceAlpha_Tint) {
  readParam(int, Dest, 0);
  readParam(Pixel, Color, 5);
  contextCheck(Dest);
  lockCheck(Dest);
  selectContext(Dest);
  setBlendMode<SourceAlpha>();
  setTextureColor(White);
  setVertexColor(White);
  setFogColor(Color);
  setFogOpacity(Color[::Alpha] / 255.0f);
  enableFog();
  BlitSimple_Core(Parameters);
  disableFog();
  return Success;
}

passOverride(BlitSimple_SourceAlpha_Solid_Tint, BlitSimple_SourceAlpha_Tint)

defOverride(BlitSimple_SourceAlpha_Tint_Opacity) {
  readParam(int, Dest, 0);
  readParam(Pixel, Color, 5);
  readParam(int, Opacity, 6);
  contextCheck(Dest);
  lockCheck(Dest);
  selectContext(Dest);
  setBlendMode<SourceAlpha>();
  setTextureColor(White);
  setVertexColor(Pixel(255, 255, 255, Opacity));
  setFogColor(Color);
  setFogOpacity(Color[::Alpha] / 255.0f);
  enableFog();
  BlitSimple_Core(Parameters);
  disableFog();
  return Success;
}

defOverride(BlitSimple_Font_SourceAlpha) {
  readParam(int, Dest, 0);
  readParam(int, Color, 5);
  contextCheck(Dest);
  lockCheck(Dest);
  selectContext(Dest);
  setBlendMode<Font_SourceAlpha>();
  setBlendColor(White);
  setTextureColorN<0>(White);
  setVertexColor(Color);
  BlitSimple_Core(Parameters);
  return Success;
}

passOverride(BlitSimple_Font_SourceAlpha_RGB, BlitSimple_Font_SourceAlpha)

defOverride(BlitSimple_Font_SourceAlpha_Opacity) {
  readParam(int, Dest, 0);
  readParam(int, Color, 5);
  readParam(int, Opacity, 6);
  contextCheck(Dest);
  lockCheck(Dest);
  selectContext(Dest);
  setBlendMode<Font_SourceAlpha>();
  setBlendColor(White);
  setTextureColorN<0>(White);
  setVertexColor(MultiplyAlpha(Color, Opacity));
  BlitSimple_Core(Parameters);
  return Success;
}

passOverride(BlitSimple_Font_SourceAlpha_RGB_Opacity, BlitSimple_Font_SourceAlpha_Opacity)

passOverride(BlitSimple_Font_Merge_RGB, BlitSimple_Font_SourceAlpha)

passOverride(BlitSimple_Font_Merge_RGB_Opacity, BlitSimple_Font_SourceAlpha_Opacity)

void BlitResample_Core(Override::OverrideParameters *Parameters) {
  readParam(int, Dest, 0);
  readParam(int, Source, 1);
  readParamRect(DestRect, 2, Null);
  readParamRect(SourceRect, 3, Null);
  readParam(int, Scaler, 4);
  FX::Rectangle DestCopy = DestRect;
  FX::Rectangle SourceCopy = SourceRect;
  if (Clip2D_PairedRect(&DestCopy, &SourceCopy, Dest, Source, &DestRect, &SourceRect, 0)) {
    enableTextures();
    selectImageAsTexture(Source);
	setScaler(Scaler);
    Texture* tex = getTexture(Source);
    if ((SourceRect.Width == tex->Width) && (SourceRect.Height == tex->Height) && (SourceRect.Left == 0) && (SourceRect.Top == 0)) { 
      drawTexturedRectangle(DestRect, tex->U1, tex->V1, tex->U2, tex->V2);
    } else {
      float U1 = tex->U(SourceRect.Left), V1 = tex->V(SourceRect.Top);
      float U2 = tex->U(SourceRect.right()), V2 = tex->V(SourceRect.bottom());
      drawTexturedRectangle(DestRect, U1, V1, U2, V2);
    }
    SetImageDirty(Dest, 1);
  }
}

defOverride(BlitResample_Normal) {
  readParam(int, Dest, 0);
  contextCheck(Dest);
  lockCheck(Dest);
  selectContext(Dest);
  setBlendMode<Normal>();
  setVertexColor(White);
  setBlendColor(White);
  BlitResample_Core(Parameters);
  return Success;
}

defOverride(BlitResample_Normal_Opacity) {
  readParam(int, Dest, 0);
  readParam(int, Opacity, 5);
  contextCheck(Dest);
  lockCheck(Dest);
  selectContext(Dest);
  setBlendMode<Normal>();
  setVertexColor(White);
  setBlendColor(Pixel(255, 255, 255, Opacity));
  BlitResample_Core(Parameters);
  return Success;
}

defOverride(BlitResample_SourceAlpha) {
  readParam(int, Dest, 0);
  contextCheck(Dest);
  lockCheck(Dest);
  selectContext(Dest);
  setBlendMode<SourceAlpha>();
  setVertexColor(White);
  BlitResample_Core(Parameters);
  return Success;
}

defOverride(BlitResample_SourceAlpha_Opacity) {
  readParam(int, Dest, 0);
  readParam(int, Opacity, 5);
  contextCheck(Dest);
  lockCheck(Dest);
  selectContext(Dest);
  setBlendMode<SourceAlpha>();
  setVertexColor(Pixel(255, 255, 255, Opacity));
  BlitResample_Core(Parameters);
  return Success;
}

defOverride(BlitResample_Additive) {
  readParam(int, Dest, 0);
  contextCheck(Dest);
  lockCheck(Dest);
  selectContext(Dest);
  setBlendMode<Additive>();
  setVertexColor(White);
  setBlendColor(White);
  BlitResample_Core(Parameters);
  return Success;
}

defOverride(BlitResample_Additive_Opacity) {
  readParam(int, Dest, 0);
  readParam(int, Opacity, 5);
  contextCheck(Dest);
  lockCheck(Dest);
  selectContext(Dest);
  setBlendMode<Additive>();
  setVertexColor(White);
  setBlendColor(Pixel(255, 255, 255, Opacity));
  BlitResample_Core(Parameters);
  return Success;
}

defOverride(BlitResample_Subtractive) {
  readParam(int, Dest, 0);
  contextCheck(Dest);
  lockCheck(Dest);
  selectContext(Dest);
  setBlendMode<Subtractive>();
  setVertexColor(White);
  setBlendColor(White);
  BlitResample_Core(Parameters);
  return Success;
}

defOverride(BlitResample_Subtractive_Opacity) {
  readParam(int, Dest, 0);
  readParam(int, Opacity, 5);
  contextCheck(Dest);
  lockCheck(Dest);
  selectContext(Dest);
  setBlendMode<Subtractive>();
  setVertexColor(White);
  setBlendColor(Pixel(255, 255, 255, Opacity));
  BlitResample_Core(Parameters);
  return Success;
}

void BlitMask_Core(Override::OverrideParameters *Parameters) {
  readParam(int, Dest, 0);
  readParam(int, Source, 1);
  readParam(int, Mask, 2);
  readParamRect(Area, 3, Null);
  readParam(int, SourceX, 4);
  readParam(int, SourceY, 5);
  readParam(int, MaskX, 6);
  readParam(int, MaskY, 7);
  FX::Rectangle AreaCopy = Area;
  if (Clip2D_SimpleRect(&Area, Dest, Source, &AreaCopy, SourceX, SourceY)) {
    int maskWidth = GetImageWidth(Mask);
    int maskHeight = GetImageHeight(Mask);
    Area.Width = ClipValue(Area.Width, 0, maskWidth - MaskX);
    Area.Height = ClipValue(Area.Height, 0, maskHeight - MaskY);
    int texWidth = powerOfTwo(GetImageWidth(Source));
    int texHeight = powerOfTwo(GetImageHeight(Source));
    float xScale = 1.0f / texWidth;
    float yScale = 1.0f / texHeight;
    float U1 = (SourceX) * xScale, V1 = (SourceY) * yScale;
    float U2 = U1 + ((Area.Width) * xScale), V2 = V1 + ((Area.Height) * yScale);
    texWidth = powerOfTwo(GetImageWidth(Mask));
    texHeight = powerOfTwo(GetImageHeight(Mask));
    xScale = 1.0f / texWidth;
    yScale = 1.0f / texHeight;
    float U3 = (MaskX) * xScale, V3 = (MaskY) * yScale;
    float U4 = U3 + (Area.Width * xScale), V4 = V3 + (Area.Height * yScale);
    enableTexture<0>();
    enableTexture<1>();
    selectImageAsTextureN<0>(Source);
    selectImageAsTextureN<1>(Mask);
    selectImageAsTextureN<0>(Source);
    selectImageAsTextureN<1>(Mask);
    setScaleMode<Linear>();
    draw2TexturedRectangle(Area, U1, V1, U2, V2, U3, V3, U4, V4);
    SetImageDirty(Dest, 1);
  }
}

defOverride(BlitMask_Normal_Opacity) {
  readParam(int, Dest, 0);
  readParam(int, Opacity, 8);
  contextCheck(Dest);
  lockCheck(Dest);
  selectContext(Dest);
  enableTexture<0>();
  setTextureColor(Pixel(255, 255, 255, Opacity));
  enableTexture<1>();
  setTextureColor(Pixel(255, 255, 255, Opacity));
  setBlendMode<Mask_Normal>();
  setVertexColor(White);
  BlitMask_Core(Parameters);
  disableTexture<1>();
  switchTextureStage<0>();
  return Success;
}

defOverride(BlitMask_SourceAlpha_Opacity) {
  readParam(int, Dest, 0);
  readParam(int, Opacity, 8);
  contextCheck(Dest);
  lockCheck(Dest);
  selectContext(Dest);
  enableTexture<0>();
  setTextureColor(Pixel(255, 255, 255, Opacity));
  enableTexture<1>();
  setTextureColor(Pixel(255, 255, 255, Opacity));
  setBlendMode<Mask_SourceAlpha>();
  setVertexColor(White);
  BlitMask_Core(Parameters);
  disableTexture<1>();
  switchTextureStage<0>();
  return Success;
}

passOverride(BlitMask_Merge_Opacity, BlitMask_SourceAlpha_Opacity)

defOverride(FilterSimple_Gradient_4Point) {
  readParam(int, Image, 0);
  contextCheck(Image);
  lockCheck(Image);
  readParamRect(Area, 1, Image);
  readParam(Pixel, ColorTL, 2);
  readParam(Pixel, ColorTR, 3);
  readParam(Pixel, ColorBL, 4);
  readParam(Pixel, ColorBR, 5);
  selectContext(Image);
  clipCheck(Image, Area);
  disableTextures();
  setBlendMode<Normal>();
  setBlendColor(White);
  drawGradientRectangle(Area, ColorTL, ColorTR, ColorBL, ColorBR);
  SetImageDirty(Image, 1);
  return Success;
}

defOverride(FilterSimple_Gradient_4Point_SourceAlpha) {
  readParam(int, Image, 0);
  contextCheck(Image);
  lockCheck(Image);
  readParamRect(Area, 1, Image);
  readParam(Pixel, ColorTL, 2);
  readParam(Pixel, ColorTR, 3);
  readParam(Pixel, ColorBL, 4);
  readParam(Pixel, ColorBR, 5);
  selectContext(Image);
  clipCheck(Image, Area);
  disableTextures();
  setBlendMode<SourceAlpha>();
  setBlendColor(White);
  drawGradientRectangle(Area, ColorTL, ColorTR, ColorBL, ColorBR);
  SetImageDirty(Image, 1);
  return Success;
}

defOverride(FilterSimple_Gradient_Horizontal) {
  readParam(int, Image, 0);
  contextCheck(Image);
  lockCheck(Image);
  readParamRect(Area, 1, Image);
  readParam(Pixel, ColorR, 2);
  readParam(Pixel, ColorL, 3);
  selectContext(Image);
  clipCheck(Image, Area);
  disableTextures();
  setBlendMode<Normal>();
  setBlendColor(White);
  drawGradientRectangle(Area, ColorL, ColorR, ColorL, ColorR);
  SetImageDirty(Image, 1);
  return Success;
}

defOverride(FilterSimple_Gradient_Horizontal_SourceAlpha) {
  readParam(int, Image, 0);
  contextCheck(Image);
  lockCheck(Image);
  readParamRect(Area, 1, Image);
  readParam(Pixel, ColorR, 2);
  readParam(Pixel, ColorL, 3);
  selectContext(Image);
  clipCheck(Image, Area);
  disableTextures();
  setBlendMode<SourceAlpha>();
  setBlendColor(White);
  drawGradientRectangle(Area, ColorL, ColorR, ColorL, ColorR);
  SetImageDirty(Image, 1);
  return Success;
}

defOverride(FilterSimple_Gradient_Vertical) {
  readParam(int, Image, 0);
  contextCheck(Image);
  lockCheck(Image);
  readParamRect(Area, 1, Image);
  readParam(Pixel, ColorB, 2);
  readParam(Pixel, ColorT, 3);
  selectContext(Image);
  clipCheck(Image, Area);
  disableTextures();
  setBlendMode<Normal>();
  setBlendColor(White);
  drawGradientRectangle(Area, ColorT, ColorT, ColorB, ColorB);
  SetImageDirty(Image, 1);
  return Success;
}

defOverride(FilterSimple_Gradient_Vertical_SourceAlpha) {
  readParam(int, Image, 0);
  contextCheck(Image);
  lockCheck(Image);
  readParamRect(Area, 1, Image);
  readParam(Pixel, ColorB, 2);
  readParam(Pixel, ColorT, 3);
  selectContext(Image);
  clipCheck(Image, Area);
  disableTextures();
  setBlendMode<SourceAlpha>();
  setBlendColor(White);
  drawGradientRectangle(Area, ColorT, ColorT, ColorB, ColorB);
  SetImageDirty(Image, 1);
  return Success;
}

defOverride(FilterSimple_Gradient_Radial) {
  readParam(int, Image, 0);
  contextCheck(Image);
  lockCheck(Image);
  readParamRect(Area, 1, Image);
  readParam(Pixel, Color1, 2);
  readParam(Pixel, Color2, 3);
  selectContext(Image);
  disableTextures();
  setBlendMode<Normal>();
  setBlendColor(White);
  setVertexColor(Color2);
  drawRectangle(Area);
  enableTextures();
  Global->GenerateRadialImage();
  selectImageAsTexture(Global->RadialImage);
  setBlendMode<SourceAlpha>();
  setVertexColor(Color1);
  float w = Area.Width / 2.0f, h = Area.Height / 2.0f;
  float x1 = Area.Left, y1 = Area.Top;
  float x2 = Area.Left + w, y2 = Area.Top + h;
  drawTexturedRectangleF(x1, y1, w, h, 1, 1, 0, 0);
  drawTexturedRectangleF(x2, y1, w, h, 0, 1, 1, 0);
  drawTexturedRectangleF(x1, y2, w, h, 1, 0, 0, 1);
  drawTexturedRectangleF(x2, y2, w, h, 0, 0, 1, 1);
  SetImageDirty(Image, 1);
  return Success;
}

defOverride(FilterSimple_Gradient_Radial_SourceAlpha) {
  readParam(int, Image, 0);
  contextCheck(Image);
  lockCheck(Image);
  readParamRect(Area, 1, Image);
  readParam(Pixel, Color1, 2);
  readParam(Pixel, Color2, 3);
  selectContext(Image);
  disableTextures();
  setBlendMode<SourceAlpha>();
  setBlendColor(White);
  setVertexColor(Color2);
  drawRectangle(Area);
  enableTextures();
  Global->GenerateRadialImage();
  selectImageAsTexture(Global->RadialImage);
  setBlendMode<SourceAlpha>();
  setScaleMode<Linear>();
  setVertexColor(Color1);
  float w = Area.Width / 2.0f, h = Area.Height / 2.0f;
  float x1 = Area.Left, y1 = Area.Top;
  float x2 = Area.Left + w, y2 = Area.Top + h;
  drawTexturedRectangleF(x1, y1, w, h, 1, 1, 0, 0);
  drawTexturedRectangleF(x2, y1, w, h, 0, 1, 1, 0);
  drawTexturedRectangleF(x1, y2, w, h, 1, 0, 0, 1);
  drawTexturedRectangleF(x2, y2, w, h, 0, 0, 1, 1);
  SetImageDirty(Image, 1);
  return Success;
}

defOverride(SetPixel) {
  readParam(int, Image, 0);
  contextCheck(Image);
  lockCheck(Image);
  readParam(int, X, 1);
  readParam(int, Y, 2);
  readParam(Pixel, Color, 3);
  selectContext(Image);
  disableTextures();
  setBlendMode<Normal>();
  setBlendColor(White);
  setVertexColor(Color);
  drawPixel(X, Y);
  SetImageDirty(Image, 1);
  return Success;
}

defOverride(SetPixelAA) {
  readParam(int, Image, 0);
  contextCheck(Image);
  lockCheck(Image);
  readParam(int, Xi, 1);
  readParam(int, Yi, 2);
  readParam(int, Xf, 3);
  readParam(int, Yf, 4);
  readParam(Pixel, Color, 5);
  selectContext(Image);
  disableTextures();
  setBlendMode<SourceAlpha>();
  setBlendColor(White);
  setVertexColor(Color);
  glEnable(GL_POINT_SMOOTH);
  drawPixel((float)Xi + ((float)Xf / 255.0f) + 0.5f, (float)Yi + ((float)Yf / 255.0f) + 0.5f);
  glDisable(GL_POINT_SMOOTH);
  SetImageDirty(Image, 1);
  return Success;
}

defOverride(FilterSimple_Line) {
  readParam(int, Image, 0);
  contextCheck(Image);
  lockCheck(Image);
  readParamRect(Area, 1, Image);
  readParam(Pixel, Color, 2);
  FPoint start, end;
  Area.getLine(start, end);
  selectContext(Image);
  disableTextures();
  setBlendMode<Normal>();
  setBlendColor(White);
  setVertexColor(Color);
  drawLine(start, end);
  SetImageDirty(Image, 1);
  return Success;
}

defOverride(FilterSimple_Line_SourceAlpha) {
  readParam(int, Image, 0);
  contextCheck(Image);
  lockCheck(Image);
  readParamRect(Area, 1, Image);
  readParam(Pixel, Color, 2);
  FPoint start, end;
  Area.getLine(start, end);
  selectContext(Image);
  disableTextures();
  setBlendMode<SourceAlpha>();
  setBlendColor(White);
  setVertexColor(Color);
  drawLine(start, end);
  SetImageDirty(Image, 1);
  return Success;
}

defOverride(FilterSimple_Line_AA) {
  readParam(int, Image, 0);
  contextCheck(Image);
  lockCheck(Image);
  readParam(float, X1, 1);
  readParam(float, Y1, 2);
  readParam(float, X2, 3);
  readParam(float, Y2, 4);
  readParam(Pixel, Color, 5);
  selectContext(Image);
  disableTextures();
  setBlendMode<SourceAlpha>();
  setBlendColor(White);
  setVertexColor(Color);
  glEnable(GL_LINE_SMOOTH);
  drawLine(FPoint(X1 + 0.5f, Y1 + 0.5f), FPoint(X2 + 0.5f, Y2 + 0.5f));
  glDisable(GL_LINE_SMOOTH);
  SetImageDirty(Image, 1);
  return Success;
}

defOverride(FilterSimple_Line_Gradient_AA) {
  readParam(int, Image, 0);
  contextCheck(Image);
  lockCheck(Image);
  readParam(float, X1, 1);
  readParam(float, Y1, 2);
  readParam(float, X2, 3);
  readParam(float, Y2, 4);
  readParam(Pixel, StartColor, 5);
  readParam(Pixel, EndColor, 6);
  selectContext(Image);
  disableTextures();
  setBlendMode<SourceAlpha>();
  setBlendColor(White);
  glEnable(GL_LINE_SMOOTH);
  drawGradientLine(FPoint(X1 + 0.5f, Y1 + 0.5f), FPoint(X2 + 0.5f, Y2 + 0.5f), StartColor, EndColor);
  glDisable(GL_LINE_SMOOTH);
  SetImageDirty(Image, 1);
  return Success;
}

defOverride(FilterSimple_Line_Additive) {
  readParam(int, Image, 0);
  contextCheck(Image);
  lockCheck(Image);
  readParamRect(Area, 1, Image);
  readParam(Pixel, Color, 2);
  FPoint start, end;
  Area.getLine(start, end);
  selectContext(Image);
  disableTextures();
  setBlendMode<Additive>();
  setBlendColor(White);
  setVertexColor(Color);
  drawLine(start, end);
  SetImageDirty(Image, 1);
  return Success;
}

defOverride(FilterSimple_Line_Subtractive) {
  readParam(int, Image, 0);
  contextCheck(Image);
  lockCheck(Image);
  readParamRect(Area, 1, Image);
  readParam(Pixel, Color, 2);
  FPoint start, end;
  Area.getLine(start, end);
  selectContext(Image);
  disableTextures();
  setBlendMode<Subtractive>();
  setBlendColor(White);
  setVertexColor(Color);
  drawLine(start, end);
  SetImageDirty(Image, 1);
  return Success;
}

defOverride(FilterSimple_Line_Gradient) {
  readParam(int, Image, 0);
  contextCheck(Image);
  lockCheck(Image);
  readParamRect(Area, 1, Image);
  readParam(Pixel, ColorS, 2);
  readParam(Pixel, ColorE, 3);
  FPoint start, end;
  Area.getLine(start, end);
  selectContext(Image);
  disableTextures();
  setBlendMode<Normal>();
  setBlendColor(White);
  drawGradientLine(start, end, ColorS, ColorE);
  SetImageDirty(Image, 1);
  return Success;
}

defOverride(FilterSimple_Line_Gradient_SourceAlpha) {
  readParam(int, Image, 0);
  contextCheck(Image);
  lockCheck(Image);
  readParamRect(Area, 1, Image);
  readParam(Pixel, ColorS, 2);
  readParam(Pixel, ColorE, 3);
  FPoint start, end;
  Area.getLine(start, end);
  selectContext(Image);
  disableTextures();
  setBlendMode<SourceAlpha>();
  setBlendColor(White);
  drawGradientLine(start, end, ColorS, ColorE);
  SetImageDirty(Image, 1);
  return Success;
}

defOverride(FilterSimple_Box) {
  readParam(int, Image, 0);
  contextCheck(Image);
  lockCheck(Image);
  readParamRect(Area, 1, Image);
  readParam(Pixel, Color, 2);
  selectContext(Image);
  clipCheck(Image, Area);
  disableTextures();
  setBlendMode<Normal>();
  setVertexColor(Color);
  setBlendColor(White);
  drawBox(Area);
  SetImageDirty(Image, 1);
  return Success;
}

defOverride(FilterSimple_Box_SourceAlpha) {
  readParam(int, Image, 0);
  contextCheck(Image);
  lockCheck(Image);
  readParamRect(Area, 1, Image);
  readParam(Pixel, Color, 2);
  selectContext(Image);
  clipCheck(Image, Area);
  disableTextures();
  setBlendMode<SourceAlpha>();
  setVertexColor(Color);
  setBlendColor(White);
  drawBox(Area);
  SetImageDirty(Image, 1);
  return Success;
}

defOverride(FilterSimple_Adjust) {
  readParam(int, Image, 0);
  contextCheck(Image);
  lockCheck(Image);
  readParamRect(Area, 1, Image);
  readParam(int, Amount, 2);
  selectContext(Image);
  clipCheck(Image, Area);
  disableTextures();
  if (Amount > 0) {
    setBlendMode<Additive>();
    setVertexColor(Pixel(Amount, Amount, Amount, 255));
  } else if (Amount < 0) {
    setBlendMode<Subtractive>();
    setVertexColor(Pixel(-Amount, -Amount, -Amount, 255));
  }
  setBlendColor(White);
  drawRectangle(Area);
  SetImageDirty(Image, 1);
  return Success;
}

defOverride(FilterSimple_Adjust_RGB) {
  readParam(int, Image, 0);
  contextCheck(Image);
  lockCheck(Image);
  readParamRect(Area, 1, Image);
  readParam(int, RedAmount, 2);
  readParam(int, GreenAmount, 3);
  readParam(int, BlueAmount, 4);
  selectContext(Image);
  clipCheck(Image, Area);
  disableTextures();
  int Amount;
  Pixel Color;
  for (int c = 0; c < 3; ++c) {
    switch (c) {
      case ::Blue:
        Amount = BlueAmount;
        break;
      case ::Green:
        Amount = GreenAmount;
        break;
      case ::Red:
        Amount = RedAmount;
        break;
    }
    Color = Pixel(0, 0, 0, 255);
    if (Amount > 0) {
      setBlendMode<Additive>();
      Color[c] = Amount;
    } else {
      setBlendMode<Subtractive>();
      Color[c] = -Amount;
    }
    setVertexColor(Color);
    setBlendColor(White);
    drawRectangle(Area);
  }
  SetImageDirty(Image, 1);
  return Success;
}

defOverride(FilterSimple_Adjust_Channel) {
  readParam(int, Image, 0);
  contextCheck(Image);
  lockCheck(Image);
  readParamRect(Area, 1, Image);
  readParam(int, Channel, 2);
  readParam(int, Amount, 3);
  selectContext(Image);
  clipCheck(Image, Area);
  disableTextures();
  Pixel Color = Pixel(0, 0, 0, 255);
  if (Amount > 0) {
    setBlendMode<Additive>();
    Color[Channel] = Amount;
  } else {
    setBlendMode<Subtractive>();
    Color[Channel] = -Amount;
  }
  setVertexColor(Color);
  setBlendColor(White);
  drawRectangle(Area);
  SetImageDirty(Image, 1);
  return Success;
}

defOverride(FilterSimple_ConvexPolygon) {
  readParam(int, Image, 0);
  readParam(int, Polygon, 1);
  readParam(Pixel, Color, 2);
  readParam(int, Renderer, 3);
  readParam(Pixel, RenderArgument, 4);
  contextCheck(Image);
  lockCheck(Image);
  selectContext(Image);
  disableTextures();
  setRenderer(Renderer, RenderArgument);
  setVertexColor(Color);
  FPoint* ptr = GetPolygonVertexPointer(Polygon, 0);
  int vertex_count = GetPolygonVertexCount(Polygon);
  if (vertex_count == 4) {
    beginDraw(GL_QUADS);
  } else if (vertex_count == 3) {
    beginDraw(GL_TRIANGLES);
  } else {
    beginDraw(GL_POLYGON);
  }
  for (int i = 0; i < vertex_count; ++i) {
    glVertex2f(ptr->X, ptr->Y);
    ptr++;
  }
  if (vertex_count > 4) endDraw();
  disableFog();
  SetImageDirty(Image, 1);
  return Success;
}

defOverride(FilterSimple_ConvexPolygon_Gradient) {
  readParam(int, Image, 0);
  readParam(int, Polygon, 1);
  readParam(Pixel, Color, 2);
  readParam(int, Renderer, 3);
  readParam(Pixel, RenderArgument, 4);
  contextCheck(Image);
  lockCheck(Image);
  selectContext(Image);
  disableTextures();
  setRenderer(Renderer, RenderArgument);
  GradientVertex* ptr = GetGradientPolygonVertexPointer(Polygon, 0);
  int vertex_count = GetGradientPolygonVertexCount(Polygon);
  if (vertex_count == 4) {
    beginDraw(GL_QUADS);
  } else if (vertex_count == 3) {
    beginDraw(GL_TRIANGLES);
  } else {
    beginDraw(GL_POLYGON);
  }
  for (int i = 0; i < vertex_count; ++i) {
    setVertexColor(ptr->Color);
    glVertex2f(ptr->X, ptr->Y);
    ptr++;
  }
  if (vertex_count > 4) endDraw();
  disableFog();
  SetImageDirty(Image, 1);
  return Success;
}

passOverride(FilterSimple_ConvexPolygon_AntiAlias, FilterSimple_ConvexPolygon);

defOverride(FilterSimple_ConvexPolygon_Textured) {
  readParam(int, Image, 0);
  readParam(int, TextureImage, 1);
  readParam(int, Polygon, 2);
  readParam(int, Scaler, 3);
  readParam(int, Renderer, 4);
  readParam(Pixel, RenderArgument, 5);
  contextCheck(Image);
  lockCheck(Image);
  selectContext(Image);
  enableTextures();
  selectImageAsTexture(TextureImage);
  setRenderer(Renderer, RenderArgument);
  setScaler(Scaler);
  Texture* tex = getTexture(TextureImage);
  TexturedVertex* ptr = GetTexturedPolygonVertexPointer(Polygon, 0);
  int vertex_count = GetTexturedPolygonVertexCount(Polygon);
  if (vertex_count == 4) {
    beginDraw(GL_QUADS);
  } else if (vertex_count == 3) {
    beginDraw(GL_TRIANGLES);
  } else {
    beginDraw(GL_POLYGON);
  }
  for (int i = 0; i < vertex_count; ++i) {
    glTexCoord2f(tex->U(ptr->U), tex->V(ptr->V));
    glVertex2f(ptr->X, ptr->Y);
    ptr++;
  }
  if (vertex_count > 4) endDraw();
  disableFog();
  SetImageDirty(Image, 1);
  return Success;
}

passOverride(FilterSimple_ConvexPolygon_Textured_AntiAlias, FilterSimple_ConvexPolygon_Textured);

defOverride(Copy) {
  readParam(int, Dest, 0);
  readParam(int, Source, 1);
  contextCheck(Source);
  if (GetImageLocked(Source)) {
    UnlockImage(Source);
    FX::Rectangle rect = FX::Rectangle(0, 0, GetImageWidth(Source), GetImageHeight(Source));
    ReallocateImage(Dest, rect.Width, rect.Height);
    BlitSimple_Normal(Dest, Source, &rect, 0, 0);
    LockImage(Source);
    SetImageDirty(Dest, 1);
    return Success;
  }
  return Failure;
}

void BlitTiled(int Dest, int Source, FX::Rectangle* DestRect) {
  if (ClipRectangle_ImageClipRect(DestRect, Dest)) {
    enableTextures();
    selectImageAsTexture(Source);
    Texture* tex = getTexture(Source);
    setScaleMode<Linear>();
    drawTexturedRectangleTiledF(DestRect->Left, DestRect->Top, DestRect->Width, DestRect->Height, tex->Width, tex->Height, tex->U1, tex->V1, tex->U2, tex->V2);
  }
}

void BlitScaled(int Dest, int Source, FX::Rectangle* DestRect) {
  if (ClipRectangle_ImageClipRect(DestRect, Dest)) {
    enableTextures();
    selectImageAsTexture(Source);
    setScaleMode<Linear>();
    Texture* tex = getTexture(Source);
    drawTexturedRectangleF(DestRect->Left, DestRect->Top, DestRect->Width, DestRect->Height, tex->U1, tex->V1, tex->U2, tex->V2);
  }
}

defOverride(RenderWindow) {
FX::Rectangle dest, source, clipper, clip;
int xs = 0, ys = 0;
int xm[2] = {0,0}, ym[2] = {0,0};
  readParam(int, Image, 0);
  contextCheck(Image);
  lockCheck(Image);
  readParamRect(Area, 1, Image);
  readParam(WindowSkinParam*, WindowSkin, 2);
  readParam(wsSectionFlags, SectionFlags, 3);
  selectContext(Image);
  GetImageClipRectangle(Image, &clip);
  clipper = clip;
  clipper.Left = ClipValue(Area.Left - WindowSkin->EdgeOffsets[0], clip.Left, clip.right());
  clipper.setRight(ClipValue(Area.right() + WindowSkin->EdgeOffsets[2], clip.Left, clip.right()));
  clipper.Top = ClipValue(Area.Top - WindowSkin->EdgeOffsets[1], clip.Top, clip.bottom());
  clipper.setBottom(ClipValue(Area.bottom() + WindowSkin->EdgeOffsets[3], clip.Top, clip.bottom()));
  xm[0] = _Max(_Max(GetImageWidth(WindowSkin->pImages[wsTopLeft]), GetImageWidth(WindowSkin->pImages[wsLeft])),GetImageWidth(WindowSkin->pImages[wsBottomLeft]));
  xm[1] = _Max(_Max(GetImageWidth(WindowSkin->pImages[wsTopRight]), GetImageWidth(WindowSkin->pImages[wsRight])),GetImageWidth(WindowSkin->pImages[wsBottomRight]));
  ym[0] = _Max(_Max(GetImageHeight(WindowSkin->pImages[wsTopLeft]), GetImageHeight(WindowSkin->pImages[wsTop])),GetImageHeight(WindowSkin->pImages[wsTopRight]));
  ym[1] = _Max(_Max(GetImageHeight(WindowSkin->pImages[wsBottomLeft]), GetImageHeight(WindowSkin->pImages[wsBottom])),GetImageHeight(WindowSkin->pImages[wsBottomRight]));
  xs = GetImageWidth(WindowSkin->pImages[wsMiddle]);
  ys = GetImageHeight(WindowSkin->pImages[wsMiddle]);
  switch(WindowSkin->RenderMode) {
  case BlitMode_Additive:
      setBlendMode<Additive>();
      break;
  case BlitMode_Subtractive:
      setBlendMode<Subtractive>();
      break;
  case BlitMode_SourceAlpha:
      setBlendMode<SourceAlpha>();
      break;
  case BlitMode_Font_SourceAlpha:
      setBlendMode<Font_SourceAlpha>();
      break;
  case BlitMode_Normal:
      setBlendMode<Normal>();
      break;
  default:
  case BlitMode_Default:
      return Failure;
      break;
  }
  SetImageClipRectangle(Image, &clipper);
  if (SectionFlags & sfMiddle) {
    GetImageRectangle(WindowSkin->pImages[wsMiddle], &source);
    dest.setValues(Area.Left - WindowSkin->EdgeOffsets[0], Area.Top - WindowSkin->EdgeOffsets[1], 
        Area.Width + WindowSkin->EdgeOffsets[0] + WindowSkin->EdgeOffsets[2], Area.Height + WindowSkin->EdgeOffsets[1] + WindowSkin->EdgeOffsets[3]);
    setFog(WindowSkin->TintColors[wsMiddle]);
    switch (WindowSkin->BackgroundMode) {
    default:
    case 0:
        // tiled blit
        setVertexColor(Pixel(255, 255, 255, WindowSkin->Alpha));
        BlitTiled(Image, WindowSkin->pImages[wsMiddle], &dest);
        break;
    case 1:
        // scaled blit
        setVertexColor(Pixel(255, 255, 255, WindowSkin->Alpha));
        BlitScaled(Image, WindowSkin->pImages[wsMiddle], &dest);
        break;
    case 2:
        // gradient
        disableTextures();
        drawGradientRectangle(&dest, MultiplyAlpha(WindowSkin->CornerColors[0], WindowSkin->Alpha), MultiplyAlpha(WindowSkin->CornerColors[1], WindowSkin->Alpha), MultiplyAlpha(WindowSkin->CornerColors[2], WindowSkin->Alpha), MultiplyAlpha(WindowSkin->CornerColors[3], WindowSkin->Alpha));
        break;
    case 3:
        // tiled blit
        setVertexColor(Pixel(255, 255, 255, WindowSkin->Alpha));
        BlitTiled(Image, WindowSkin->pImages[wsMiddle], &dest);
        disableTextures();
        drawGradientRectangle(&dest, MultiplyAlpha(WindowSkin->CornerColors[0], WindowSkin->Alpha), MultiplyAlpha(WindowSkin->CornerColors[1], WindowSkin->Alpha), MultiplyAlpha(WindowSkin->CornerColors[2], WindowSkin->Alpha), MultiplyAlpha(WindowSkin->CornerColors[3], WindowSkin->Alpha));
        break;
    case 4:
        // scaled blit
        setVertexColor(Pixel(255, 255, 255, WindowSkin->Alpha));
        BlitScaled(Image, WindowSkin->pImages[wsMiddle], &dest);
        disableTextures();
        drawGradientRectangle(&dest, MultiplyAlpha(WindowSkin->CornerColors[0], WindowSkin->Alpha), MultiplyAlpha(WindowSkin->CornerColors[1], WindowSkin->Alpha), MultiplyAlpha(WindowSkin->CornerColors[2], WindowSkin->Alpha), MultiplyAlpha(WindowSkin->CornerColors[3], WindowSkin->Alpha));
        break;
    }
  }
  setVertexColor(Pixel(255, 255, 255, WindowSkin->Alpha));
  SetImageClipRectangle(Image, &clip);
  if (SectionFlags & sfTop) {
    setFog(WindowSkin->TintColors[wsTop]);
    dest.setValues(Area.Left, Area.Top - GetImageHeight(WindowSkin->pImages[wsTop]), Area.Width, GetImageHeight(WindowSkin->pImages[wsTop]));
    BlitTiled(Image, WindowSkin->pImages[wsTop], &dest);
  }
  if (SectionFlags & sfBottom) {
    setFog(WindowSkin->TintColors[wsBottom]);
    dest.setValues(Area.Left, Area.bottom(), Area.Width, GetImageHeight(WindowSkin->pImages[wsBottom]));
    BlitTiled(Image, WindowSkin->pImages[wsBottom], &dest);
  }
  if (SectionFlags & sfLeft) {
    setFog(WindowSkin->TintColors[wsLeft]);
    dest.setValues(Area.Left - GetImageWidth(WindowSkin->pImages[wsLeft]), Area.Top, GetImageWidth(WindowSkin->pImages[wsLeft]), Area.Height);
    BlitTiled(Image, WindowSkin->pImages[wsLeft], &dest);
  }
  if (SectionFlags & sfRight) {
    setFog(WindowSkin->TintColors[wsRight]);
    dest.setValues(Area.right(), Area.Top, GetImageWidth(WindowSkin->pImages[wsRight]), Area.Height);
    BlitTiled(Image, WindowSkin->pImages[wsRight], &dest);
  }
  if (SectionFlags & sfBottomRight) {
    setFog(WindowSkin->TintColors[wsBottomRight]);
    dest.setValues(Area.right(), Area.bottom(), GetImageWidth(WindowSkin->pImages[wsBottomRight]), GetImageHeight(WindowSkin->pImages[wsBottomRight]));
    BlitScaled(Image, WindowSkin->pImages[wsBottomRight], &dest);
  }
  if (SectionFlags & sfBottomLeft) {
    setFog(WindowSkin->TintColors[wsBottomLeft]);
    dest.setValues(Area.Left - GetImageWidth(WindowSkin->pImages[wsBottomLeft]), Area.bottom(), GetImageWidth(WindowSkin->pImages[wsBottomLeft]), GetImageHeight(WindowSkin->pImages[wsBottomLeft]));
    BlitScaled(Image, WindowSkin->pImages[wsBottomLeft], &dest);
  }
  if (SectionFlags & sfTopRight) {
    setFog(WindowSkin->TintColors[wsTopRight]);
    dest.setValues(Area.right(), Area.Top - GetImageHeight(WindowSkin->pImages[wsTopRight]), GetImageWidth(WindowSkin->pImages[wsTopRight]), GetImageHeight(WindowSkin->pImages[wsTopRight]));
    BlitScaled(Image, WindowSkin->pImages[wsTopRight], &dest);
  }
  if (SectionFlags & sfTopLeft) {
    setFog(WindowSkin->TintColors[wsTopLeft]);
    dest.setValues(Area.Left - GetImageWidth(WindowSkin->pImages[wsTopLeft]), Area.Top - GetImageHeight(WindowSkin->pImages[wsTopLeft]), GetImageWidth(WindowSkin->pImages[wsTopLeft]), GetImageHeight(WindowSkin->pImages[wsTopLeft]));
    BlitScaled(Image, WindowSkin->pImages[wsTopLeft], &dest);
  }
  disableFog();
  SetImageDirty(Image, 1);
  return Success;
}

/*
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
          FilterSimple_Gradient_4Point_SourceAlpha(Dest, &dest, MultiplyAlpha(wp->CornerColors[0], wp->Alpha), MultiplyAlpha(wp->CornerColors[1], wp->Alpha), MultiplyAlpha(wp->CornerColors[2], wp->Alpha), MultiplyAlpha(wp->CornerColors[3], wp->Alpha));
          break;
      case 3:
          if ((xs <= 1) && (ys <= 1)) {
              // alpha fill
              FilterSimple_Fill_SourceAlpha_Opacity(Dest, &dest, wp->pImages[wsMiddle]->getPixel(0,0), wp->Alpha);
          } else {
              // tiled blit
              ModedTiledBlit((SFX_BlitModes)wp->RenderMode, Dest, wp->pImages[wsMiddle], &dest, wp->TintColors[wsMiddle], wp->Alpha);
          }
          FilterSimple_Gradient_4Point_SourceAlpha(Dest, &dest, MultiplyAlpha(wp->CornerColors[0], wp->Alpha), MultiplyAlpha(wp->CornerColors[1], wp->Alpha), MultiplyAlpha(wp->CornerColors[2], wp->Alpha), MultiplyAlpha(wp->CornerColors[3], wp->Alpha));
          break;
      case 4:
          if ((xs <= 1) && (ys <= 1)) {
              // alpha fill
              FilterSimple_Fill_SourceAlpha_Opacity(Dest, &dest, wp->pImages[wsMiddle]->getPixel(0,0), wp->Alpha);
          } else {
              // scaled blit
              ModedResampleBlit((SFX_BlitModes)wp->RenderMode, Dest, wp->pImages[wsMiddle], &dest, &source, wp->TintColors[wsMiddle], wp->Alpha);
          }
          FilterSimple_Gradient_4Point(Dest, &dest, MultiplyAlpha(wp->CornerColors[0], wp->Alpha), MultiplyAlpha(wp->CornerColors[1], wp->Alpha), MultiplyAlpha(wp->CornerColors[2], wp->Alpha), MultiplyAlpha(wp->CornerColors[3], wp->Alpha));
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
*/

defOverride(RenderTilemapLayer) {
int tile = 0, lasttile = -1;
int cx = 0, cy = 0;
short *pRow, *pTile;
FX::Rectangle rctDest;
FX::Rectangle oldRect;
int pTarget = 0;
int maxX = 0, maxY = 0;
int cv = 0;
Texture* tex;
  readParam(TilemapLayerParam*, Layer, 0);
  readParam(CameraParam*, Camera, 1);
  readParam(int, sx, 2);
  readParam(int, sy, 3);
  readParam(int, ex, 4);
  readParam(int, ey, 5);
  readParam(int, camerax, 6);
  readParam(int, cameray, 7);
  readParam(int, alpha, 8);

  pTarget = Camera->pImage();
  if (Layer->RenderTarget < Camera->RenderTargetCount) {
    pTarget = Camera->pRenderTargets[Layer->RenderTarget];
  }

  contextCheck(pTarget);
  lockCheck(pTarget);
  selectContext(pTarget);

  maxX = Layer->Width - 1;
  maxY = Layer->Height - 1;
  
  int tileCount = GetTileCount(Layer->pTileset);
  int tileWidth = GetTileWidth(Layer->pTileset);
  int tileHeight = GetTileHeight(Layer->pTileset);

  switch(Layer->Effect) {
  default:
  case 0:
    setBlendMode<Normal>();
    setVertexColor(White);
    setBlendColor(Pixel(255, 255, 255, alpha));
    if (Layer->TintColor[::Alpha] > 0) {
      enableFog();
      setFogColor(Layer->TintColor);
      setFogOpacity(Layer->TintColor[::Alpha] / 255.0f);
    }
    break;
  case 1:
  case 2:
    setBlendMode<SourceAlpha>();
    setVertexColor(Pixel(255, 255, 255, alpha));
    if (Layer->TintColor[::Alpha] > 0) {
      enableFog();
      setFogColor(Layer->TintColor);
      setFogOpacity(Layer->TintColor[::Alpha] / 255.0f);
    }
    break;
  case 3:
    setBlendMode<Additive>();
    setVertexColor(White);
    setBlendColor(Pixel(255, 255, 255, alpha));
    break;
  case 4:
    setBlendMode<Subtractive>();
    setVertexColor(White);
    setBlendColor(Pixel(255, 255, 255, alpha));
    break;
  case 7:
    setBlendMode<Multiply>();
    setVertexColor(White);
    setTextureColor(Pixel(alpha, alpha, alpha, alpha));
    break;
  case 8:
    setBlendMode<Lightmap>();
    setVertexColor(White);
    setTextureColor(Pixel(alpha, alpha, alpha, alpha));
    break;
  }

  enableTextures();

  // initialize the y coordinate
  float tx, ty, tw = tileWidth, th = tileHeight;
  ty = -cameray;
  for (cy = sy; cy < ey; ++cy) {
      tx = -camerax;
      if (Layer->WrapY) {
          pRow = Layer->pData + (Layer->Width * (cy % maxY));
      } else {
          pRow = Layer->pData + (Layer->Width * cy);
      }
      pTile = pRow + sx;
      for (cx = sx; cx < ex; ++cx) {
        if (Layer->WrapX) {
            pTile = pRow + (cx % maxX);
        }
        cv = *pTile;
        if ((cv == Layer->MaskedTile) || (cv >= tileCount) || (cv < 0)) {
        } else {
          tile = GetTileFast(Layer->pTileset, cv, Layer->pAnimationMap);
          if (tile != lasttile) {
            selectImageAsTexture(tile);
            if (Layer->Effect == 1) prepMatte(tile);
            tex = getTexture(tile);
            lasttile = tile;
          }
          drawTexturedRectangleF(tx, ty, tw, th, tex->U1, tex->V1, tex->U2, tex->V2);
        }
        tx += tw;
        pTile++;
      }
      ty += th;
  }
  SetImageDirty(pTarget, 1);
  disableFog();
  return Success;
}

defOverride(Deallocate) {
	readParam(int, Image, 0);
  if (GetImagePointer(Image, 0, 0) != 0) {
    Texture* tex = getTexture(Image);
    if (tex) {
      for (unsigned int i = 0; i < Global->SmallImageCache.size(); ++i) {
        Global->SmallImageCache[i]->freeSpot(tex);
      }
      delete getTexture(Image);
      setNamedTag(Image, Texture, 0);
    }
    for (unsigned int i = 0; i < Global->ImageHeap.size(); ++i) {
      if (Global->ImageHeap[i] == Image) {
        Global->ImageHeap[i] = 0;
      }
    }
	  if (checkNamedTag(Image, Context)) {
      HGLRC context = (HGLRC)getNamedTag(Image, Context);
      if (context != 0) {
        flushImageHeap();
        wglMakeCurrent(Global->DC, 0);
        wglDeleteContext(context);
        if (Global->Framebuffer == Image) {
          Global->Framebuffer = 0;
        }
        SetImageLocked(Image, 0);
      }
      Global->Context = 0;
	  }
  }
	return Failure;
}

defOverride(BlitDeform) {
  readParam(int, Dest, 0);
  readParam(int, Source, 1);
  readParam(MeshParam*, Mesh, 2);
  readParamRect(DestRect, 3, Null);
  readParamRect(SourceRect, 4, Null);
  readParam(Pixel, RenderArgument, 5);
  readParam(int, Renderer, 6);
  readParam(int, Scaler, 7);
  contextCheck(Dest);
  lockCheck(Dest);
  selectContext(Dest);
  //clipCheck(Dest, DestRect);
  enableTextures();
  selectImageAsIsolatedTexture(Source);
  selectImageAsIsolatedTexture(Source);
  setRenderer(Renderer, RenderArgument);
  setScaler(Scaler);
  DoubleWord iCX = 0, iCY = DestRect.Height + 1;  
  int pixelsToDraw = 0;
  int mw = Mesh->Width - 1, mh = Mesh->Height - 1;
  if ((mw < 1) || (mh < 1)) return Failure;
  int cx = 0, cy = 0;
  float cxw = 0, cyw = 0, sx = 0, sy = 0, lsx = 0, lsy = 0;
  float cxi = (mw / (float)DestRect.Width), cyi = (mh / (float)DestRect.Height);
  float dx = DestRect.Left, dy = DestRect.Top;
  bool update_points = true, first_update = true, perform_draw = false;
  Texture* tex = getTexture(Source);
  if (tex->IsolatedTexture != 0) tex = tex->IsolatedTexture;
  float bx = SourceRect.Left, by = SourceRect.Top;
  float bxi = (1 / (DestRect.Width / (float)(SourceRect.Width))), byi = (1 / (DestRect.Height / (float)(SourceRect.Height)));
  MeshPoint p[4];
  beginDraw(GL_LINES);
  while (iCY--) {
    iCX = (DoubleWord)DestRect.Width + 1;
    cxw = 0;
    cx = 0;
    bx = SourceRect.Left;
    dx = DestRect.Left;
    update_points = true;
    first_update = true;
    pixelsToDraw = 0;
    while (iCX--) {
      cxw += cxi;
      pixelsToDraw++;
      while (cxw >= 1.0f) {
        cxw -= 1.0f;
        cx++;
        update_points = true;
      }
      if ((update_points) || (iCX == 0)) {
        update_points = false;
        lsx = sx;
        lsy = sy;
        /* sometimes i hate that inline is just a hint
        Mesh->get4Points(cx, cy, p);
        this is one of those times */
        
        p[0] = (Mesh->pData[ClipValue(cx, 0, mw) + (ClipValue(cy, 0, mh) * Mesh->Width)]);
        p[1] = (Mesh->pData[ClipValue(cx+1, 0, mw) + (ClipValue(cy, 0, mh) * Mesh->Width)]);
        p[2] = (Mesh->pData[ClipValue(cx, 0, mw) + (ClipValue(cy+1, 0, mh) * Mesh->Width)]);
        p[3] = (Mesh->pData[ClipValue(cx+1, 0, mw) + (ClipValue(cy+1, 0, mh) * Mesh->Width)]);

        sx = bx + (((p[0].X * (1 - cxw) + p[1].X * cxw) * (1 - cyw) + (p[2].X * (1 - cxw) + p[3].X * cxw) * cyw));
        sy = by + (((p[0].Y * (1 - cxw) + p[1].Y * cxw) * (1 - cyw) + (p[2].Y * (1 - cxw) + p[3].Y * cxw) * cyw)) - 0.5;
        if (first_update) {
          first_update = false;
          bx += bxi;
        }
        else
          perform_draw = true;
      }
      if (perform_draw) {
        if (iCX == 0) pixelsToDraw--;
        perform_draw = false;
        glTexCoord2f(tex->U(lsx), tex->V(lsy));
        glVertex2f(dx, dy);
        glTexCoord2f(tex->U(sx), tex->V(sy));
        glVertex2f(dx + pixelsToDraw, dy);
        dx += pixelsToDraw;
        pixelsToDraw = 0;
      }
      bx += bxi;
    }
    by += byi;
    cyw += cyi;
    dy += 1.0f;
    while (cyw >= 1.0f) {
      cyw -= 1.0f;
      cy++;
      update_points = true;
    }
  }
  endDraw();
  setTextureColorN<0>(White);
  setTextureColorN<1>(White);
  disableTextures();
  return Success;
}

defOverride(BlitDeformMask) {
  readParam(int, Dest, 0);
  readParam(int, Source, 1);
  readParam(int, Mask, 2);
  readParam(MeshParam*, Mesh, 3);
  readParamRect(DestRect, 4, Null);
  readParamRect(SourceRect, 5, Null);
  readParamRect(MaskRect, 6, Null);
  readParam(Pixel, RenderArgument, 7);
  readParam(int, Opacity, 8);
  readParam(int, Renderer, 9);
  readParam(int, Scaler, 10);
  contextCheck(Dest);
  lockCheck(Dest);
  selectContext(Dest);
  //clipCheck(Dest, DestRect);
  enableTexture<0>();
  enableTexture<1>();
  selectImageAsIsolatedTextureN<0>(Source);
  selectImageAsTextureN<1>(Mask);
  selectImageAsIsolatedTextureN<0>(Source);
  setTextureColor(Pixel(255, 255, 255, Opacity));
  selectImageAsTextureN<1>(Mask);
  setTextureColor(Pixel(255, 255, 255, Opacity));
  setMaskRenderer(Renderer, RenderArgument);
  setScaler(Scaler);
  DoubleWord iCX = 0, iCY = DestRect.Height + 1;  
  int pixelsToDraw = 0;
  int mw = Mesh->Width - 1, mh = Mesh->Height - 1;
  if ((mw < 1) || (mh < 1)) return Failure;
  int cx = 0, cy = 0;
  float cxw = 0, cyw = 0, sx = 0, sy = 0, lsx = 0, lsy = 0;
  float cxi = (mw / (float)DestRect.Width), cyi = (mh / (float)DestRect.Height);
  float dx = DestRect.Left, dy = DestRect.Top;
  bool update_points = true, first_update = true, perform_draw = false;
  Texture* tex = getTexture(Source);
  Texture* mtex = getTexture(Mask);
  if (tex->IsolatedTexture != 0) tex = tex->IsolatedTexture;
  float bx = SourceRect.Left, by = SourceRect.Top;
  float bxi = (1 / (DestRect.Width / (float)(SourceRect.Width))), byi = (1 / (DestRect.Height / (float)(SourceRect.Height)));
  MeshPoint p[4];
  beginDraw(GL_LINES);
  while (iCY--) {
    iCX = (DoubleWord)DestRect.Width + 1;
    cxw = 0;
    cx = 0;
    bx = SourceRect.Left;
    dx = DestRect.Left;
    update_points = true;
    first_update = true;
    pixelsToDraw = 0;
    while (iCX--) {
      cxw += cxi;
      pixelsToDraw++;
      while (cxw >= 1.0f) {
        cxw -= 1.0f;
        cx++;
        update_points = true;
      }
      if ((update_points) || (iCX == 0)) {
        update_points = false;
        lsx = sx;
        lsy = sy;
        /* sometimes i hate that inline is just a hint
        Mesh->get4Points(cx, cy, p);
        this is one of those times */
        
        p[0] = (Mesh->pData[ClipValue(cx, 0, mw) + (ClipValue(cy, 0, mh) * Mesh->Width)]);
        p[1] = (Mesh->pData[ClipValue(cx+1, 0, mw) + (ClipValue(cy, 0, mh) * Mesh->Width)]);
        p[2] = (Mesh->pData[ClipValue(cx, 0, mw) + (ClipValue(cy+1, 0, mh) * Mesh->Width)]);
        p[3] = (Mesh->pData[ClipValue(cx+1, 0, mw) + (ClipValue(cy+1, 0, mh) * Mesh->Width)]);

        sx = bx + (((p[0].X * (1 - cxw) + p[1].X * cxw) * (1 - cyw) + (p[2].X * (1 - cxw) + p[3].X * cxw) * cyw));
        sy = by + (((p[0].Y * (1 - cxw) + p[1].Y * cxw) * (1 - cyw) + (p[2].Y * (1 - cxw) + p[3].Y * cxw) * cyw)) - 0.5;
        if (first_update) {
          first_update = false;
          bx += bxi;
        }
        else
          perform_draw = true;
      }
      if (perform_draw) {
        if (iCX == 0) pixelsToDraw--;
        perform_draw = false;
        glMultiTexCoord2fARB(GL_TEXTURE0_ARB, tex->U(lsx), tex->V(lsy));
        glMultiTexCoord2fARB(GL_TEXTURE1_ARB, mtex->U(dx + MaskRect.Left), mtex->V(dy + MaskRect.Top - 1));
        glVertex2f(dx, dy);
        glMultiTexCoord2fARB(GL_TEXTURE0_ARB, tex->U(sx), tex->V(sy));
        glMultiTexCoord2fARB(GL_TEXTURE1_ARB, mtex->U(dx + pixelsToDraw + MaskRect.Left), mtex->V(dy + MaskRect.Top - 1));
        glVertex2f(dx + pixelsToDraw, dy);
        dx += pixelsToDraw;
        pixelsToDraw = 0;
      }
      bx += bxi;
    }
    by += byi;
    cyw += cyi;
    dy += 1.0f;
    while (cyw >= 1.0f) {
      cyw -= 1.0f;
      cy++;
      update_points = true;
    }
  }
  endDraw();
  setTextureColorN<0>(White);
  setTextureColorN<1>(White);
  disableTextures();
  return Success;
}

Export void GLRenderFunction(int Dest, int X, int Y, void *Source, int Count) {
  if (Global->RenderTexture) {
    endDraw();
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, Count, 1, GL_BGRA_EXT, GL_UNSIGNED_BYTE, Source);
    GL::drawTexturedLine(X, Y + 1, X + Count, Y + 1, Global->RenderTexture->U(0), Global->RenderTexture->V(0), Global->RenderTexture->U(Count), Global->RenderTexture->V(0));
  }
}

defOverride(GetScanlineRenderer) {
  readParam(int, Dest, 0);
  readParam(int, Renderer, 1);
  readParam(int, Count, 2);
  readParam(Pixel, Argument, 3);
  contextCheck(Dest);
  lockCheck(Dest);
  selectContext(Dest);
  if (Global->RenderTexture) {
    if ((Global->RenderTexture->Width < Count) || (Global->RenderTexture->isInvalid())) {
      delete Global->RenderTexture;
      Global->RenderTexture = 0;
    }
  }
  if (Global->RenderTexture) {
  } else {
    Global->RenderTexture = GL::createTexture(Count + 1, 2, false);
  }
  Global->RenderFunction = Renderer;
  setRenderer(Renderer, Argument);
  setVertexColor(White);
  enableTextures();
  selectTexture(Global->RenderTexture->Handle);
  return reinterpret_cast<int>(GLRenderFunction);
}

void InstallOverrides() {
	addOverride(Deallocate);
  addOverride(Clear);
  addOverride(Copy);
  addOverride(Lock);
  addOverride(Unlock);
  addOverride(SetPixel);
  addOverride(SetPixelAA);
  addOverride(FilterSimple_Fill);
  addOverride(FilterSimple_Fill_Opacity);
  addOverride(FilterSimple_Fill_SourceAlpha);
  addOverride(FilterSimple_Fill_SourceAlpha_Opacity);
  addOverride(FilterSimple_Fill_Additive);
  addOverride(FilterSimple_Fill_Additive_Opacity);
  addOverride(FilterSimple_Fill_Subtractive);
  addOverride(FilterSimple_Fill_Subtractive_Opacity);
  addOverride(BlitSimple_Normal);
  addOverride(BlitSimple_Normal_Opacity);
  addOverride(BlitSimple_Normal_Tint);
  addOverride(BlitSimple_Normal_Tint_Opacity);
  addOverride(BlitSimple_Matte);
  addOverride(BlitSimple_Matte_Opacity);
  addOverride(BlitSimple_Matte_Tint);
  addOverride(BlitSimple_Matte_Tint_Opacity);
  addOverride(BlitSimple_SourceAlpha);
  addOverride(BlitSimple_SourceAlpha_Opacity);
  addOverride(BlitSimple_SourceAlphaMatte);
  addOverride(BlitSimple_SourceAlphaMatte_Opacity);
  addOverride(BlitSimple_SourceAlpha_Tint);
  addOverride(BlitSimple_SourceAlpha_Solid_Tint);
  addOverride(BlitSimple_SourceAlpha_Tint_Opacity);
  addOverride(BlitSimple_Font_SourceAlpha);
  addOverride(BlitSimple_Font_SourceAlpha_Opacity);
  addOverride(BlitSimple_Font_SourceAlpha_RGB);
  addOverride(BlitSimple_Font_SourceAlpha_RGB_Opacity);
  addOverride(BlitSimple_Additive);
  addOverride(BlitSimple_Additive_Opacity);
  addOverride(BlitSimple_Multiply);
  addOverride(BlitSimple_Multiply_Opacity);
  addOverride(BlitSimple_Lightmap);
  addOverride(BlitSimple_Lightmap_Opacity);
  addOverride(BlitSimple_Lightmap_RGB);
  addOverride(BlitSimple_Lightmap_RGB_Opacity);
  addOverride(BlitSimple_Subtractive);
  addOverride(BlitSimple_Subtractive_Opacity);
  addOverride(BlitSimple_Additive_SourceAlpha);
  addOverride(BlitSimple_Additive_SourceAlpha_Opacity);
  addOverride(BlitSimple_Subtractive_SourceAlpha);
  addOverride(BlitSimple_Subtractive_SourceAlpha_Opacity);
  addOverride(BlitSimple_Merge);
  addOverride(BlitSimple_Merge_Opacity);
  addOverride(BlitSimple_Font_Merge_RGB);
  addOverride(BlitSimple_Font_Merge_RGB_Opacity);
  addOverride(BlitResample_Normal);
  addOverride(BlitResample_Normal_Opacity);
  addOverride(BlitResample_SourceAlpha);
  addOverride(BlitResample_SourceAlpha_Opacity);
  addOverride(BlitResample_Additive);
  addOverride(BlitResample_Additive_Opacity);
  addOverride(BlitResample_Subtractive);
  addOverride(BlitResample_Subtractive_Opacity);
  addOverride(BlitMask_Normal_Opacity);
  addOverride(BlitMask_SourceAlpha_Opacity);
  addOverride(BlitMask_Merge_Opacity);
  addOverride(BlitDeform);
  addOverride(BlitDeformMask);
  addOverride(FilterSimple_Gradient_4Point);
  addOverride(FilterSimple_Gradient_4Point_SourceAlpha);
  addOverride(FilterSimple_Gradient_Vertical);
  addOverride(FilterSimple_Gradient_Vertical_SourceAlpha);
  addOverride(FilterSimple_Gradient_Horizontal);
  addOverride(FilterSimple_Gradient_Horizontal_SourceAlpha);
  addOverride(FilterSimple_Gradient_Radial);
  addOverride(FilterSimple_Gradient_Radial_SourceAlpha);
  addOverride(FilterSimple_Line);
  addOverride(FilterSimple_Line_SourceAlpha);
  addOverride(FilterSimple_Line_Additive);
  addOverride(FilterSimple_Line_Subtractive);
  addOverride(FilterSimple_Line_Gradient);
  addOverride(FilterSimple_Line_Gradient_SourceAlpha);
  addOverride(FilterSimple_Line_AA);
  addOverride(FilterSimple_Line_Gradient_AA);
  addOverride(FilterSimple_Box);
  addOverride(FilterSimple_Box_SourceAlpha);
  addOverride(FilterSimple_Adjust);
  addOverride(FilterSimple_Adjust_RGB);
  addOverride(FilterSimple_Adjust_Channel);
  addOverride(FilterSimple_ConvexPolygon);
  addOverride(FilterSimple_ConvexPolygon_Textured);
  addOverride(FilterSimple_ConvexPolygon_Gradient);
//  addOverride(FilterSimple_ConvexPolygon_AntiAlias);
//  addOverride(FilterSimple_ConvexPolygon_Textured_AntiAlias);
  addOverride(RenderTilemapLayer);
  addOverride(RenderWindow);
  addOverride(GetScanlineRenderer);
}

void UninstallOverrides() {
	removeOverride(Deallocate);
  removeOverride(Clear);
  removeOverride(Copy);
  removeOverride(Lock);
  removeOverride(Unlock);
  removeOverride(SetPixel);
  removeOverride(SetPixelAA);
  removeOverride(FilterSimple_Fill);
  removeOverride(FilterSimple_Fill_Opacity);
  removeOverride(FilterSimple_Fill_SourceAlpha);
  removeOverride(FilterSimple_Fill_SourceAlpha_Opacity);
  removeOverride(FilterSimple_Fill_Additive);
  removeOverride(FilterSimple_Fill_Additive_Opacity);
  removeOverride(FilterSimple_Fill_Subtractive);
  removeOverride(FilterSimple_Fill_Subtractive_Opacity);
  removeOverride(BlitSimple_Normal);
  removeOverride(BlitSimple_Normal_Opacity);
  removeOverride(BlitSimple_Normal_Tint);
  removeOverride(BlitSimple_Normal_Tint_Opacity);
  removeOverride(BlitSimple_Matte);
  removeOverride(BlitSimple_Matte_Opacity);
  removeOverride(BlitSimple_Matte_Tint);
  removeOverride(BlitSimple_Matte_Tint_Opacity);
  removeOverride(BlitSimple_SourceAlpha);
  removeOverride(BlitSimple_SourceAlpha_Opacity);
  removeOverride(BlitSimple_SourceAlphaMatte);
  removeOverride(BlitSimple_SourceAlphaMatte_Opacity);
  removeOverride(BlitSimple_SourceAlpha_Tint);
  removeOverride(BlitSimple_SourceAlpha_Solid_Tint);
  removeOverride(BlitSimple_SourceAlpha_Tint_Opacity);
  removeOverride(BlitSimple_Font_SourceAlpha);
  removeOverride(BlitSimple_Font_SourceAlpha_Opacity);
  removeOverride(BlitSimple_Font_SourceAlpha_RGB);
  removeOverride(BlitSimple_Font_SourceAlpha_RGB_Opacity);
  removeOverride(BlitSimple_Additive);
  removeOverride(BlitSimple_Additive_Opacity);
  removeOverride(BlitSimple_Multiply);
  removeOverride(BlitSimple_Multiply_Opacity);
  removeOverride(BlitSimple_Lightmap);
  removeOverride(BlitSimple_Lightmap_Opacity);
  removeOverride(BlitSimple_Lightmap_RGB);
  removeOverride(BlitSimple_Lightmap_RGB_Opacity);
  removeOverride(BlitSimple_Subtractive);
  removeOverride(BlitSimple_Subtractive_Opacity);
  removeOverride(BlitSimple_Additive_SourceAlpha);
  removeOverride(BlitSimple_Additive_SourceAlpha_Opacity);
  removeOverride(BlitSimple_Subtractive_SourceAlpha);
  removeOverride(BlitSimple_Subtractive_SourceAlpha_Opacity);
  removeOverride(BlitSimple_Merge);
  removeOverride(BlitSimple_Merge_Opacity);
  removeOverride(BlitSimple_Font_Merge_RGB);
  removeOverride(BlitSimple_Font_Merge_RGB_Opacity);
  removeOverride(BlitResample_Normal);
  removeOverride(BlitResample_Normal_Opacity);
  removeOverride(BlitResample_SourceAlpha);
  removeOverride(BlitResample_SourceAlpha_Opacity);
  removeOverride(BlitResample_Additive);
  removeOverride(BlitResample_Additive_Opacity);
  removeOverride(BlitResample_Subtractive);
  removeOverride(BlitResample_Subtractive_Opacity);
  removeOverride(BlitMask_Normal_Opacity);
  removeOverride(BlitMask_SourceAlpha_Opacity);
  removeOverride(BlitMask_Merge_Opacity);
  removeOverride(BlitDeform);
  removeOverride(BlitDeformMask);
  removeOverride(FilterSimple_Gradient_4Point);
  removeOverride(FilterSimple_Gradient_4Point_SourceAlpha);
  removeOverride(FilterSimple_Gradient_Vertical);
  removeOverride(FilterSimple_Gradient_Vertical_SourceAlpha);
  removeOverride(FilterSimple_Gradient_Horizontal);
  removeOverride(FilterSimple_Gradient_Horizontal_SourceAlpha);
  removeOverride(FilterSimple_Gradient_Radial);
  removeOverride(FilterSimple_Gradient_Radial_SourceAlpha);
  removeOverride(FilterSimple_Line);
  removeOverride(FilterSimple_Line_SourceAlpha);
  removeOverride(FilterSimple_Line_Additive);
  removeOverride(FilterSimple_Line_Subtractive);
  removeOverride(FilterSimple_Line_Gradient);
  removeOverride(FilterSimple_Line_Gradient_SourceAlpha);
  removeOverride(FilterSimple_Line_AA);
  removeOverride(FilterSimple_Line_Gradient_AA);
  removeOverride(FilterSimple_Box);
  removeOverride(FilterSimple_Box_SourceAlpha);
  removeOverride(FilterSimple_Adjust);
  removeOverride(FilterSimple_Adjust_RGB);
  removeOverride(FilterSimple_Adjust_Channel);
  removeOverride(FilterSimple_ConvexPolygon);
  removeOverride(FilterSimple_ConvexPolygon_Textured);
  removeOverride(FilterSimple_ConvexPolygon_Gradient);
//  removeOverride(FilterSimple_ConvexPolygon_AntiAlias);
//  removeOverride(FilterSimple_ConvexPolygon_Textured_AntiAlias);
  removeOverride(RenderTilemapLayer);
  removeOverride(RenderWindow);
  removeOverride(GetScanlineRenderer);
}

Export void GLInstallAllocateHook() {
  addOverride(Allocate);
  return;
}

Export void GLUninstallAllocateHook() {
  removeOverride(Allocate);
  return;
}
