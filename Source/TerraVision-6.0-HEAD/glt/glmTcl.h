#include "glt.h"
#include "glm.h"
#include "togl.h"

#if defined(HAVE_X11)
#include "glmx.h"
extern void fixupGLXContext(glmContext, GLXContext, XVisualInfo*);

#elif defined(HAVE_WINDOWS_H)
extern void fixupGLXContext(glmContext, ??????);
#endif

extern glmContext glmTclCreateContext(struct Togl *);

extern glmWindow glmTclCreateWindow(struct Togl*, glmContext, 
    int doubleBuffer, int stereo);

extern void glmTclRedrawDispatch(glmWindow, void* closure);
