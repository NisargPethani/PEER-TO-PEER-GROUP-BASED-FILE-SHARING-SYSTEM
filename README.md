# Terminal Based File Explorer

## Objective:

Build a fully functional File Explorer Application, albeit with a restricted feature set.

## Prerequisites:

* Basic usage and architectural know-how of file explorer features
* Preliminaries such as C/C++ code compilation, execution & debugging

## Requirements:

Your File Explorer should work in two modes -

* Normal mode (default mode) - used to explore the current directory and navigate the filesystem
* Command mode - used to enter shell commands
	* The root of your application should be the directory where the application was started.
	* The application should display data starting from the top-left corner of the terminal window, line-by-line. You should be able to handle text rendering if the terminal window is resized. The last line of the display screen is to be used as a status bar.

## Normal mode:

Normal mode is the default mode of your application. It should have the following functionalities -

* Display a list of directories and files in the current folder
	* Every file in the directory should be displayed on a new line with the following attributes. for each file:
		* File Name
		* File Size
		* Ownership (user and group) and Permissions
		* Last modified
		* All of this should be displayed in human readable format
	* The file explorer should show entries “.” and “..” for current and parent directory respectively
	* The file explorer should handle scrolling in the case of vertical overflow using keys ​ **k** ​ & ​ **l**
	* User should be able to navigate up and down in the file list using the corresponding up and down arrow keys


* Open directories and files. When enter key is pressed :
	* Directory - Clear the screen and navigate into the directory and show the directory contents as specified in point 1
	* File - Open the file in vi editor
* Traversal
	* Go back - Left arrow key should take the user to the previously visited directory
	* Go forward - Right arrow key should take the user to the next directory
	* Up one level - Backspace key should take the user up one level
	* Home - ​ **h** ​ key should take the user to the home folder (the folder where the application was started)

### Command Mode:

The application should enter the Command button whenever “:” (colon) key is pressed. In the command
mode, the user should be able to enter different commands. All commands appear in the status bar at
the bottom.

* Copy - ‘copy <source_file(s)> <destination_directory>’
Move - ‘move <source_file(s)> <destination_directory>’
Rename - ‘rename <old_filename> <new_filename>’
	* Eg - ​copy foo.txt bar.txt baz.mp4 ~/foobar
	* move foo.txt bar.txt baz.mp4 ~/foobar
	* rename foo.txt bar.txt
	* Assume that the destination directory exists and you have write permissions.
	* Copying/Moving directories should also be implemented
	* The file ownership and permissions should remain intact
* Create File - ‘create_file <file_name> <destination_path>’
Create Directory - ‘create_dir <dir_name> <destination_path>’
	* Eg -​ ​create_file foo.txt ~/foobar
	* create_file foo.txt.
	* create_dir foo ~/foobar
* Delete File - ‘delete_file <file_path>’
Delete Directory - ‘delete_dir <dir_path>’
	* The file/dir path should be relative to the root from where the application is run
* Goto - ‘goto <location>’
	* Eg - ​goto <directory_path>
	* Absolute path wrt application root will be given
* Search - ‘search <file_name>’ or ‘search <directory_name>’
	* Search for a given file or folder under the current directory recursively
	* Output should be True or False depending on whether the file or folder exists
* On pressing ​ **ESC** ​ key, the application should go back to Normal Mode

## Guidelines:

* Languages allowed: C/C++. Use of STL is allowed.
* Use of system() library function for command mode functions is not allowed.
* Use of system commands like ls, cp, mv, mkdir etc are not allowed. You have to implement your own versions of these commands.
* Use of ncurses.h is strictly prohibited.


