# cld-ssh

cld-ssg is a simple static blog generator I created for generating my personal homepage. It's written in pure C, relies on the excellent [cmark](https://github.com/commonmark/cmark) library for parsing Commonmark and dirent.h functionality for navigating directories.

Configuration is done by modifying the parameters in config.h. When the application is run it generates the following:

- index.html
  - The full contents of the most recent INDEX_COUNT posts
- archive.html
  - A list of all posts on the site

It also copies the contents of RESOURCESDIR into the HTMLDIR recursively, so any images/CSS/static HTML/other files you want to host should go in there.

cld-ssg is licensed under the zlib license. If you do use it, awesome! I'd love to hear what you think and see what you create with it.

