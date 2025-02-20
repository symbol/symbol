import shutil
def on_post_build(config):
    shutil.rmtree('pages/devbook/reference')