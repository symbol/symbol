import shutil
import os
from mkdocs import plugins
from mkdocs.config.defaults import MkDocsConfig
from mkdocs.structure.pages import Page

# Before i18n plugin
@plugins.event_priority(-10)
def _on_files_hide(files, config: MkDocsConfig):
    #import pdb; pdb.set_trace()
    #for f in files:
    #    print (f.src_path)
    #print(config)
    new_files = []
    excluded_files = []
    prefix = os.path.join('devbook', 'reference')
    for f in files:
        if not f.src_path.startswith(prefix):
            new_files.append(f)
        else:
            excluded_files.append(f)
    config['my_excluded_files'] = excluded_files
    return new_files

# After i18n plugin
@plugins.event_priority(-105)
def _on_files_show(files, config: MkDocsConfig):
    for f in config['my_excluded_files']:
        files.append (f)
    return files

on_files = plugins.CombinedEvent(_on_files_hide, _on_files_show)
