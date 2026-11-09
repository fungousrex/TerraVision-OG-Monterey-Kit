How to compile TerraVision with Microsoft Visual Studio V6.0

1. Open the Workspace file TerraVision\winbuild\TerraVision.dsw

2. Setup include files 
	a. In Tools -> Options select Directory.
	b. Select 'include files' in the Show Directories combo box. 
	c. Add the path to the following directories (prefixing with
           the full MS-DOS pathname to the TerraVision installation)
		TerraVision
		TerraVision\conversions
		TerraVision\ghttp
		TerraVision\glt
		TerraVision\glt\winbuild
		TerraVision\glt\winbuild\glt
		TerraVision\jpeg\winbuild
		TerraVision\libhs
		TerraVision\libungif\lib
		TerraVision\libxml
		TerraVision\libgeotiff\libxtiff
		TerraVision\nsprnew\pr\include\private
		TerraVision\nsprnew\pr\include
		TerraVision\nsprnew\winbuild
		TerraVision\pthr
		TerraVision\pthr\winbuild
		TerraVision\TerraVision\
		TerraVision\tiff\libtiff
		TerraVision\tsm
		TerraVision\tsm\rpc
		TerraVision\tsm\winbuild
		TerraVision\tsm\winbuild\tsm
		TerraVision\vrml2
		TerraVision\zlib

3. Setup library files
	a. In Tools -> Options select Directory.
	b. Select 'library files' in the Show Directories combo box. 
	c. Add the path for the following directories (prefixing with
           the full MS-DOS pathname to the TerraVision installation)
		TerraVision\bin

4. Quit and restart Visual Studio to be sure that all settings have
   been committed.

5. Set teTerraVision as active project

6. Select Rebuild All

7. Run

teTerraVision is the project file for the 'original' TerraVision that
generates the standalone terravision.exe that is used in that Netscape
plugin.

TerraVision is the project file for the ActiveX Control and generates
the terravision.ocx. TestActiveX is a test program that generates a
CView class and display the ActiveX control in that view.

