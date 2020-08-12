GARbro
======

Visual Novels resource browser.

Requires .NET Framework v4.5 or newer (https://www.microsoft.com/net)

Operation
---------
Browse through the file system to a file of interest.  If you think it's an
archive, try to 'enter' inside by pressing 'Enter' on it.  If GARbro
recognizes format its contents will be displayed just like regular file
system.  Some archives are encrypted, so you will be asked for credentials or
a supposed game title.  If game is not listed among presented options then
most likely archive could not be opened by current GARbro version.

Files could be extracted from archives by pressing 'F4', with all images and
audio converted to common formats in the process, of course if game format
itself is recognized.

When displaying file system contents GARbro assigns types to files based on
their names extension (so it's not always correct).  If types are misapplied,
it could be changed by selecting files and assigning type manually via context
menu 'Assign file type'.

Hotkeys
-------
Enter	Try to open selected file as archive -OR- playback audio file
F4	Extract from archive
F6	Convert selected media files
Ctrl-A  Select all files
Numpad+ Select files by mask
Space	Select next file
Ctrl-O  Open file dialog
Ctrl-E	Open folder in windows explorer
Ctrl-H	Expand window to fit an image
F3	Create archive
F5	Refresh view
Del	Delete files (not inside archives)
Ctrl-Q	Exit

Source and latest releases
--------------------------
~ https://github.com/morkt/GARbro
