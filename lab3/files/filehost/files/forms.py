from django import forms
from django.contrib.auth.models import User

from .models import File

class UploadFileForm(forms.ModelForm):
    error_css_class = 'error'

    class Meta:
        model = File
        fields = ['name', 'content']
        widgets = {
                'name': forms.TextInput(attrs={'class': 'form-control'})
        }

class UserForm(forms.ModelForm):
    class Meta:
        model = User
        fields = ['username', 'email', 'password']
        widgets = {
                'username': forms.TextInput(attrs={'class': 'form-control'}),
                'email': forms.TextInput(attrs={'class': 'form-control'}),
                'password': forms.PasswordInput(attrs={'class': 'form-control'})
        }
