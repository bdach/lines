# -*- coding: utf-8 -*-
from __future__ import unicode_literals

from django.http import HttpResponse
from django.core.paginator import Paginator, EmptyPage, PageNotAnInteger
from django.shortcuts import render, get_object_or_404
from .models import File

def index(request):
    file_list = File.objects.all()
    paginator = Paginator(file_list, 25)

    page = request.GET.get('page')
    try:
        files = paginator.page(page)
    except PageNotAnInteger:
        files = paginator.page(1)
    except EmptyPage:
        files = paginator.page(paginator.num_pages)

    return render(request, 'files/index.html', { "files": files })

def download(request):
    requested_file = get_object_or_404(File, pk = request.GET.get('id'))
    requested_file.content.open()
    response = HttpResponse(requested_file.content.read())
    response['Content-Disposition'] = 'attachment; filename="{}"'.format(requested_file.content.name)
    requested_file.content.close()
    return response
