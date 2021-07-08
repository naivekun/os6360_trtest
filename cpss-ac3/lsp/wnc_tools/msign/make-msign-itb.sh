#!/bin/bash

#Install msign tool
gem build msign.gemspec
export PATH=$PATH:$(ruby -rubygems -e 'puts Gem.user_dir')/bin
gem install --user-install msign-0.1.0.gem

#copy itb
mv ../../../output/wnc-diag-msign.itb .

echo "phrase=trVdLnxBW2suACV}ADD=J8" 
msign sign wnc-diag-msign.itb --keydir=keys
msign verify wnc-diag-msign.itb --keydir=keys
