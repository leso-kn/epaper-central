import datetime
import subprocess

project = "epaper-central"
copyright = str(datetime.date.today().year) + ", Lesosoftware"

html_theme = "sphinx_rtd_theme"

extensions = ['breathe']

breathe_projects = {"epaper-central": "xml"}
breathe_default_project = "epaper-central"

subprocess.call('cd ..; doxygen', shell=True)
