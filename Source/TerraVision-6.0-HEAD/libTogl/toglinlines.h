int ToglIsOverlaySupported(struct Togl* togl)
{    return togl->OverlayFlag; }

int ToglIsRGBA(struct Togl* togl)
{    return togl->RgbaFlag; }

int ToglIsDoubleBuffered(struct Togl* togl)
{    return togl->DoubleFlag; }

int ToglIsStereo(struct Togl* togl)
{    return togl->StereoFlag; }

int ToglGetColorDepth(struct Togl* togl)
{    return togl->RgbaRed + togl->RgbaGreen + togl->RgbaBlue; } /* +Alpha? */

int ToglGetZDepth(struct Togl* togl)
{    return togl->DepthSize; }

int ToglGetStencilDepth(struct Togl* togl)
{    return togl->StencilSize; }


#if 0 //RLB -- NOTE NEEDED ANYMORE?

#if defined(HAVE_X11)
GLXContext getGLContext(struct Togl* t)
{
    return t->GlCtx;
}
#endif

#if defined(WIN32)
HGLRC getGLContext(struct Togl* t)
{
    return t->tglGLHglrc;
}
#endif

#endif
