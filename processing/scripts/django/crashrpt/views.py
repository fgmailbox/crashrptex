from django import forms
from django.conf import settings
from django.http import HttpResponse, HttpResponseRedirect, Http404
from django.shortcuts import render_to_response
from django.template import RequestContext
import re
import uuid
import os
from django.views.decorators.csrf import csrf_exempt
import hashlib

test_md5 = 'lkkjsdhfakjhkjhfa097924'

class takeReportForm(forms.Form):
    """ 
    this is the form definition 
    taken from the CrashRpt the documentation
    """
    appname=forms.CharField(label="Application name:", initial="TestApp")
    appversion=forms.CharField(label="Application version:", initial="1.0.0")
    emailfrom=forms.CharField(label="Email from:",initial="aschoen@gwdg.de",required=False)
    emailsubject=forms.CharField(label="Email subject:",initial="TestApp Crashed",required=False)
    crashguid=forms.CharField(label="Crash GUID:",initial=uuid.uuid4().hex)
    md5=forms.CharField(label="MD5 Checksum:",initial=test_md5);
    crashrpt=forms.FileField(label="Attach ZIP file:")
    username=forms.CharField(label="User name:", initial="",required=False)
    computername=forms.CharField(label="Computer name:", initial="",required=False)
    domainname=forms.CharField(label="Domain name:", initial="",required=False)

def getMD5(formFileObject):
    try:
        m = hashlib.md5()
        for chunk in formFileObject:
            m.update(chunk)
        return m.hexdigest()
    except:
        return ''
    
def storeFile(formFileObject,path):
    """ 
    takes the file and stores into repository 
    It's just a torso!
    """
    try:
        destination = open(path, 'wb+')
        for chunk in formFileObject.chunks():
            destination.write(chunk)
        destination.close()
        return HttpResponse('200 Success', status=200)
    except:
        return HttpResponse('452 File storage failed', status=452)

def add_from_form(formData, s_key, s_dir):    
    if formData.has_key(s_key):
        if formData[s_key]:
            s_dir = os.path.join(s_dir, formData[s_key])
            if os.path.exists(s_dir):
                if not os.path.isdir(s_dir):
                    return ''
            else:
                try:
                    os.mkdir(s_dir)
                except:
                    return ''
            return s_dir
    
    return s_dir
                
def processReport(formData,formFileObject,addr):
    """ the main processing engine for the report """
    
    # Check the md5 hash for validity
    if formData['md5'] != test_md5:
        if getMD5(formFileObject) != formData['md5']:
            return HttpResponse('452 Invalid MD5 checksum', status=452)
    
    # If the directory from the settings does not exist, return an error
    s_dir = settings.CRASHRPT_DIR
    if not os.path.isdir(os.path.join(s_dir)):
        return HttpResponse('452 Server error', status=452)
    
    # Add appname and version as directory and create them if necessary
    s_dir = add_from_form(formData, 'appname', s_dir)
    if not s_dir:
        return HttpResponse('452 Server error', status=452)
    s_dir = add_from_form(formData, 'appversion', s_dir)
    if not s_dir:
        return HttpResponse('452 Server error', status=452)
    
    # Form a filename prefix if the computer/domain/username was given
    fn = ''
    #if formData.get('domainname'):
    #    if formData.get('computername'):
    #        if formData.get('username'):
    #            fn = '%(domainname)s - %(computername)s - %(username)s - ' % formData
    #    
    #if addr:
    #    fn += '%s - ' % addr
    fn += formData['crashguid'] + '.zip'
    
    # For now, just store the file.
    return storeFile(formFileObject, os.path.join(s_dir,fn))
    
@csrf_exempt 
def takeReport(request):
    """ takes the request and responses """
    # when POST is submitted
    if request.method == 'POST':
        # get the form data
        form=takeReportForm(request.POST,request.FILES)
        if form.is_valid():	  
            # form is valid, process the whol thin
            addr = request.META.get('REMOTE_ADDR')
                
            return processReport(form.cleaned_data,request.FILES['crashrpt'], addr)
        else:
            # when django form validation (see form definition) dails
            return HttpResponse('452 Invalid post data', status=452)
        
    # this renders a web form, when no POST data is submitted #
    else:
        form=takeReportForm()
        
    context=RequestContext(request, {'form' : form,})    
    return render_to_response('path.to.templates/crashrpt.html',context)
