#!/bin/bash

svn st $1 | grep "^?" | awk '{print $2}' | xargs -I {} svn add {}
svn commit -m "$2" $1 --username lokiwang --password Love@BaoBao3