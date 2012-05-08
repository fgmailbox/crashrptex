This is a starting point for receiving CrashRpt reports on your django based server. It is not a 
working app, rather some ideas and building blocks.

On a working django server:

(1)
The 'crashrpt' directory goes somewhere into your root folder (where the settings.py etc. reside).

(2)
In your settings file, set the variable CRASHRPT_DIR to an absolute path where your reports 
should be written

(3)
Put the files in templates into the templates directory (or, better into templates/crashrpt).

(4)
Paste the entry from urls.py into your urls.py and adjust the path to the view as needed (see 1)
and the URL as you wish.

(5)
Edit the path to the template in views.py and the path to views.py in the crashrpt.html template

(6)
Edit views.py and dependent files such that the file naming is to your liking or to kick off 
any analysis.
If you like, re-enable the filname prefixing based on crash information below the comment
# Form a filename prefix if the computer/domain/username was given


