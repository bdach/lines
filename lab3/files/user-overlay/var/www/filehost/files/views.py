# -*- coding: utf-8 -*-
from __future__ import unicode_literals

from django.contrib import messages
from django.contrib.auth import authenticate, login, logout
from django.contrib.auth.mixins import LoginRequiredMixin
from django.core.paginator import Paginator, EmptyPage, PageNotAnInteger
from django.http import HttpResponse
from django.shortcuts import redirect, render, get_object_or_404
from django.views import View
from .models import File
from .forms import UploadFileForm, UserForm

class IndexView(View):
    def get(self, request):
        file_list = File.objects.all()
        paginator = Paginator(file_list, 25)

        page = request.GET.get('page')
        try:
            files = paginator.page(page)
        except PageNotAnInteger:
            files = paginator.page(1)
        except EmptyPage:
            files = paginator.page(paginator.num_pages)

        form = UploadFileForm()

        return render(request, 'files/index.html', { "files": files, "form": form, "messages": messages.get_messages(request) })

def download(request):
    requested_file = get_object_or_404(File, pk = request.GET.get('id'))
    requested_file.content.open()
    response = HttpResponse(requested_file.content.read())
    response['Content-Disposition'] = 'attachment; filename="{}"'.format(requested_file.content.name)
    requested_file.content.close()
    return response

class UploadView(LoginRequiredMixin, View):
    login_url = '/login'

    def post(self, request):
        form = UploadFileForm(request.POST, request.FILES)
        if form.is_valid():
            form.save()
            messages.success(request, 'File uploaded successfully.')
        else:
            messages.error(request, 'There was an error uploading the file.')
        return redirect('index')

class LogOutView(LoginRequiredMixin, View):
    login_url = '/'

    def get(self, request):
        logout(request)
        return redirect('index')

class LogInView(View):
    def post(self, request):
        if request.user.is_authenticated:
            return redirect('index')
        username = request.POST['username']
        password = request.POST['password']
        user = authenticate(username=username, password=password)
        if user is not None:
            login(request, user)
            messages.success(request, 'Logged in successfully as {}'.format(user.username))
        else:
            messages.error(request, 'Login failed due to invalid credentials.')
        return redirect('index')
 
class RegisterView(View):
    def get(self, request):
        if request.user.is_authenticated:
            messages.info(request, 'Please log out before registering another account.')
            return redirect('index')
        form = UserForm()
        return render(request, 'files/register.html', {"form": form})

    def post(self, request):
        if request.user.is_authenticated:
            messages.info(request, 'Please log out before registering another account.')
            return redirect('index')
        form = UserForm(request.POST)
        if form.is_valid():
            form.save()
            messages.success(request, 'User {} registered successfully.'.format(form.cleaned_data['username']))
        else:
            messages.error(request, 'Could not register user:\n{}'.format(form.errors.join('\n')))
        return redirect('index')
