# -*- coding: utf-8 -*-
from __future__ import unicode_literals

from django.db import models

class File(models.Model):
    name = models.CharField(max_length=250)
    content = models.FileField()

    def __str__(self):
        return self.name
