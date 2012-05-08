from django.conf.urls.defaults import *
# etc. etc., your imports

urlpatterns=patterns('',
		# Your patterns
        # Add one entry for crashrpt
        (r'^[cC]rash[rR]pt/*$', 'path.to.crashrpt.views.takeReport'),
		...
		)

