6/9/2000

[These notes are written specifcally in reference to Redhat Linux
6.2., for the Intel platform.  Most likely they are applicable as well
to other Linux distributions and architectures. --RLB]

The Redhat 6.2 stock distribution comes with an RPM package for Mesa
3.1.  Redhat's installation procedure offers several convenience
installations, which install variously configured subsets of the
distribution.  It is not known whether Mesa is included if Linux is
installed as either a "workstation" or "server" type
installation. Clearly, Mesa is installed if the "everything"
installation is performed, or if appropriate care is taken during a
"custom" install.  Note at the time of this writing, the latest
version of Mesa is 3.2.

In any case, the Redhat distribution does *not* include the
GLw{,M}DrawingArea widget, upon which TerraVision and other GLX
applications depend. Thus, the widget sources must be acquired, built,
and installed before TerraVision can be compiled. Sources for these
widgets are included in the Mesa source code distribution, which may
be found at http://download.sourceforge.net/mesa3d/.  At this level of
effort, it clearly makes sense to go ahead and upgrade the entire Mesa
installation.

There are two issues to be aware of when building a Mesa distribution.
One is that the default installation directory for Mesa is /usr/local,
whereas the Redhat distribution places the Mesa/OpenGL files under
/usr/X11R6.  The Redhat scheme is easily preserved by using the
--PREFIX directive when Mesa is configured for building:
      
      ./configure --PREFIX=/usr/X11R6

Additionally, Mesa installs the widgets' libraries as:

	      libMesaGLw.a
	      libMesaGLwM.a

However, the Makefiles for TerraVision (and most OpenGL programs)
expect to find:

	     libGLw.a
	     libGLwM.a

This is easily resolved by creating links to bridge the two
namespaces.

Portions of the TerraVision build process further assume the OpenGL
include files are located under "/usr/include", as per standard IRIX
practice.  By typical Linux conventions, the OpenGL include files are
under "/usr/X11R6/include". This problem is easly solved by creating a
link such as:

	 cd /usr/include
	 ln -s /usr/X11R6/include/GL GL

