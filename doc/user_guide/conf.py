# -*- coding: utf-8 -*-
#
# Amanzi documentation build configuration file, created by
# sphinx-quickstart on Fri Mar  9 13:53:35 2012.
#
# This file is execfile()d with the current directory set to its containing dir.
#
# Note that not all possible configuration values are present in this
# autogenerated file.
#
# All configuration values have a default; values that are commented out
# serve to show the default.

import sys, os, subprocess

assert sys.version_info.major >= 3, "Python 3.x is required to build documentation"

# If extensions (or modules to document with autodoc) are in another directory,
# add these directories to sys.path here. If the directory is relative to the
# documentation root, use os.path.abspath to make it absolute, like shown here.
#sys.path.insert(0, os.path.abspath('.'))

# -- General configuration -----------------------------------------------------

# If your documentation needs a minimal Sphinx version, state it here.
#needs_sphinx = '1.0'


#
#  Add path for custom extensions
#
sys.path.append(os.path.abspath('.'))

# 
#  Extensions 
#
# 
#  Extensions 
#
ext_sphinx = ['sphinx.ext.todo', 
              'sphinx.ext.mathjax', 
              'sphinx.ext.ifconfig',
              'sphinx.ext.autodoc',
              'sphinx.ext.doctest',
              'sphinxcontrib.tikz',
              'sphinxcontrib.bibtex',
]

ext_matplotlib = ['matplotlib.sphinxext.plot_directive']

ext_ipython = ['IPython.sphinxext.ipython_directive',
               'IPython.sphinxext.ipython_console_highlighting'
]
# These moved, need to check version of ipython.
#              'matplotlib.sphinxext.ipython_directive',
#              'matplotlib.sphinxext.ipython_console_highlighting',

ext_amanzi = ['extensions.hello',
              'extensions.amanzi_xml',
]

#
# Collect extensions
#
extensions = ext_sphinx+ext_matplotlib+ext_ipython+ext_amanzi

if ( os.environ.get('MATHJAX_SSL') == "1" ):
    mathjax_path='https://software.lanl.gov/ascem/tpls/mathjax/latest/MathJax.js?config=TeX-AMS-MML_HTMLorMML'
#endif


# Add any paths that contain templates here, relative to this directory.
templates_path = ['_templates']

# The suffix of source filenames.
source_suffix = '.rst'

# The encoding of source files.
#source_encoding = 'utf-8-sig'

# The master toctree document.
master_doc = 'index'

# General information about the project.
project = u'Amanzi'
copyright = u'2016, Amanzi Development Team'

# The version info for the project you're documenting, acts as replacement for
# |version| and |release|, also used in various other places throughout the
# built documents.
#
decode = lambda x : x.decode(sys.stdout.encoding) if isinstance(x,bytes) else x

amanzi_branch=decode(subprocess.check_output('git symbolic-ref --short HEAD',shell=True).rstrip())
amanzi_global_id=decode(subprocess.check_output('git rev-parse --short HEAD',shell=True).rstrip())
amanzi_latest_tag=decode(subprocess.check_output('git tag -l \'amanzi-*\'', shell=True)).split()[-1].rstrip()
amanzi_latest_tag_ver=amanzi_latest_tag.replace('amanzi-','')

# The short X.Y version.
version = amanzi_latest_tag_ver
# The full version, including alpha/beta/rc tags.
#release = '0.80-dev'
#release = amanzi_latest_tag_ver+"\_"+amanzi_global_id
release = amanzi_latest_tag_ver

# The language for content autogenerated by Sphinx. Refer to documentation
# for a list of supported languages.
#language = None

# There are two options for replacing |today|: either, you set today to some
# non-false value, then it is used:
#today = ''
# Else, today_fmt is used as the format for a strftime call.
#today_fmt = '%B %d, %Y'

# List of patterns, relative to source directory, that match files and
# directories to ignore when looking for source files.
exclude_patterns = ['_build','testing','prototype','viz']
# exclude_patterns = ['_build','testing','install']

# The reST default role (used for this markup: `text`) to use for all documents.
#default_role = None

# If true, '()' will be appended to :func: etc. cross-reference text.
#add_function_parentheses = True

# If true, the current module name will be prepended to all description
# unit titles (such as .. function::).
#add_module_names = True

# If true, sectionauthor and moduleauthor directives will be shown in the
# output. They are ignored by default.
#show_authors = False

# The name of the Pygments (syntax highlighting) style to use.
pygments_style = 'sphinx'

# A list of ignored prefixes for module index sorting.
#modindex_common_prefix = []


# -- Options for HTML output ---------------------------------------------------

# The theme to use for HTML and HTML Help pages.  See the documentation for
# a list of builtin themes.
html_theme = 'sphinx_rtd_theme'

# Theme options are theme-specific and customize the look and feel of a theme
# further.  For a list of options available for each theme, see the
# documentation.
#html_theme_options = {}

# Add any paths that contain custom themes here, relative to this directory.
#html_theme_path = []

# The name for this set of Sphinx documents.  If None, it defaults to
# "<project> v<release> documentation".
#html_title = None

# A shorter title for the navigation bar.  Default is the same as html_title.
#html_short_title = None

# The name of an image file (relative to this directory) to place at the top
# of the sidebar.
#html_logo = None

# The name of an image file (within the static path) to use as favicon of the
# docs.  This file should be a Windows icon file (.ico) being 16x16 or 32x32
# pixels large.
#html_favicon = None

# Add any paths that contain custom static files (such as style sheets) here,
# relative to this directory. They are copied after the builtin static files,
# so a file named "default.css" will overwrite the builtin "default.css".
html_static_path = ['_static']

html_css_files = [
  'fix_eq_position.css',
]

# If not '', a 'Last updated on:' timestamp is inserted at every page bottom,
# using the given strftime format.
#html_last_updated_fmt = '%b %d, %Y'

# If true, SmartyPants will be used to convert quotes and dashes to
# typographically correct entities.
#html_use_smartypants = True

# Custom sidebar templates, maps document names to template names.
#html_sidebars = {}

# Additional templates that should be rendered to pages, maps page names to
# template names.
#html_additional_pages = {}

# If false, no module index is generated.
#html_domain_indices = True

# If false, no index is generated.
#html_use_index = True

# If true, the index is split into individual pages for each letter.
#html_split_index = False

# If true, links to the reST sources are added to the pages.
#html_show_sourcelink = True

# If true, "Created using Sphinx" is shown in the HTML footer. Default is True.
#html_show_sphinx = True

# If true, "(C) Copyright ..." is shown in the HTML footer. Default is True.
#html_show_copyright = True

# If true, an OpenSearch description file will be output, and all pages will
# contain a <link> tag referring to it.  The value of this option must be the
# base URL from which the finished HTML is served.
#html_use_opensearch = ''

# This is the file name suffix for HTML files (e.g. ".xhtml").
#html_file_suffix = None

# Output file base name for HTML help builder.
htmlhelp_basename = 'Amanzidoc'

# User control whether ''ToDo'' notes are included
if (os.environ.get('AMANZI_INCLUDE_TODOS') == "0" or os.environ.get('AMANZI_INCLUDE_TODOS') == "False" ):
    env_include_todos=False
else:
    env_include_todos=True
#endif

[extensions]
todo_include_todos=env_include_todos

# -- Options for LaTeX output --------------------------------------------------

latex_elements = {
# The paper size ('letterpaper' or 'a4paper').
'papersize': 'letterpaper',

# The font size ('10pt', '11pt' or '12pt').
'pointsize': '11pt',

# Additional stuff for the LaTeX preamble.
'preamble': '\\usepackage[version=3]{mhchem}\n\\usepackage{amssymb,grffile}\n',
}

# Grouping the document tree into LaTeX files. List of tuples
# (source start file, target name, title, author, documentclass [howto/manual]).
latex_documents = [
  ('index', 'AmanziUserGuide.tex', u'Amanzi User Guide',
   u'Amanzi Development Team (LANL, LBNL, PNNL)', 'manual'),
]

# The name of an image file (relative to this directory) to place at the top of
# the title page.
#latex_logo = None

# For "manual" documents, if this is true, then toplevel headings are parts,
# not chapters.
#latex_use_parts = False

# If true, show page references after internal links.
#latex_show_pagerefs = False

# If true, show URL addresses after external links.
#latex_show_urls = False

# Documents to append as an appendix to all manuals.
#latex_appendices = []

# If false, no module index is generated.
#latex_domain_indices = True


# -- Options for manual page output --------------------------------------------

# One entry per manual page. List of tuples
# (source start file, name, description, authors, manual section).
man_pages = [
    ('index', 'amanzi', u'Amanzi Documentation',
     [u'Amanzi Development Team (LANL, LBNL, PNNL)'], 1)
]

# If true, show URL addresses after external links.
#man_show_urls = False


# -- Options for Texinfo output ------------------------------------------------

# Grouping the document tree into Texinfo files. List of tuples
# (source start file, target name, title, author,
#  dir menu entry, description, category)
texinfo_documents = [
  ('index', 'Amanzi', u'Amanzi Documentation',
   u'Amanzi Development Team (LANL, LBNL, PNNL)', 'Amanzi', 'One line description of project.',
   'Miscellaneous'),
]

# Documents to append as an appendix to all manuals.
#texinfo_appendices = []

# If false, no module index is generated.
#texinfo_domain_indices = True

# How to display URL addresses: 'footnote', 'no', or 'inline'.
#texinfo_show_urls = 'footnote'

