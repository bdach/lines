from django.conf.urls import url
from django.contrib.auth.forms import UserCreationForm
from django.views.generic.edit import CreateView

from . import views

urlpatterns = [
        url(r'^$', views.IndexView.as_view(), name='index'),
        url(r'^download$', views.download, name='download'),
        url(r'^upload$', views.UploadView.as_view(), name='upload'),
        url(r'^login$', views.LogInView.as_view(), name='login'),
        url(r'^logout$', views.LogOutView.as_view(), name='logout'),
        url(r'^register$', CreateView.as_view(template_name="files/register.html", form_class=UserCreationForm, success_url="/"), name="register")
]
