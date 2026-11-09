/*
 * These functions have been added to the base Togl distribution to expose 
 * some of the internal configurations of the graphics context that gets
 * created at runtime.
 *
 */

extern int ToglIsOverlaySupported(struct Togl*);
extern int ToglIsRGBA(struct Togl*);
extern int ToglIsDoubleBuffered(struct Togl*);
extern int ToglIsStereo(struct Togl*);
extern int ToglGetColorDepth(struct Togl*);
extern int ToglGetZDepth(struct Togl*);
extern int ToglGetStencilDepth(struct Togl*);


#if 0 // RLB -- NOT NEEDED ANYMORE?
#if defined(HAVE_X11)
GLXContext getGLContext(struct Togl*);
#endif

#if defined(__WIN32__)
HGLRC getGLContext(struct Togl*);
#endif
#endif
