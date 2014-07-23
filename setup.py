#!/usr/bin/env python2
# -*- coding: utf-8 -*-
#
# graph-gen - https://github.com/wil93/graph-gen
# Copyright © 2014 Gabriele Farina <gabr.farina@gmail.com>
# Copyright © 2014 William Di Luigi <williamdiluigi@gmail.com>
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#    http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.


from distutils.core import setup

setup(
	name = 'graph-gen',
	version = '0.1',
	description = 'graph-gen: graph generation library for Python',
	author = 'Giorgio Audrito, William Di Luigi, Gabriele Farina',
	author_email = 'williamdiluigi@gmail.com, gabr.farina@gmail.com',
	url = 'https://github.com/wil93/graph-gen',
	py_modules = ['graphgen'],
	keywords="graph generation library",
	classifiers=[
		"Development Status :: 3 - Alpha",
		"Natural Language :: English",
		"Operating System :: OS Independent",
		"Programming Language :: Python :: 2",
		"License :: OSI Approved :: Apache Software License",
	]
)
