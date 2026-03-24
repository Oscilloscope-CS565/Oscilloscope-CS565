To View and Edit the Code:

Get the code Editor (Mac and Windows):

1. Install Visual Studio Code: https://code.visualstudio.com/download.
2. Install the C/C++ extensions for VS Code: View / Extensions and search for C/C++.
3. Click File/Open Folder and open the folder with the .c files.  You should now be able to edit and view the source code.

To compile on Windows:

Get the C/C++ Compiler: 

1. Install the Microsoft Visual C++ (MSVC) compiler:
https://visualstudio.microsoft.com/downloads/#remote-tools-for-visual-studio-2022

a) Go all the way to the bottom of the page and select: Build Tools for Visual Studio 2022
b) Check the desktop development, as shown in the Prerequisites on this page:

https://code.visualstudio.com/docs/cpp/config-msvc#_run-vs-code-outside-the-developer-command-prompt

2. In Windows Search type: Developer PowerShell and open it.
3. At the command prompt type: cl 
4. If installed correctly, you'll get a message about Compiler version and the architecture: for x86 or x64.  Remember this.

(Optional if Visual Studio Community is installed: After installation open a command prompt and run the following command:
"C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvarsall.bat" amd64 -- if you want to compile the x64 architecture)

5. Change directory to your *.c files.
6. For x86 architecture type:

cl *.c /Zi /EHsc /nologo /link i386/ftd2xx.lib /out:controller.exe

7. For x64 architecture type:

cl *.c /Zi /EHsc /nologo /link amd64/ftd2xx.lib /out:controller.exe

8. If everything works, you now have a controller.exe.  Run that from the command prompt.
9. Select Send Morse Code and test the LEDs by connecting the power wire to DB0 on the chip.

To compile on Mac:

1. Install the GCC compiler from the command line: brew install gcc
2. If (1) fails that you can install the XCode tools: xcode-select --install
3. Open the command line and change directory to the directory with the source code.  
4. From the command prompt, type: 

gcc *.c libftd2xx.a -I. -L. -framework CoreFoundation -framework IOKit -o controller

This should compile the code and return and executable called controller.
Start the controller from the command line by typing: ./controller
