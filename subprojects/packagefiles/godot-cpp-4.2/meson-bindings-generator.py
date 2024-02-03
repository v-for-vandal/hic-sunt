#!/usr/bin/env python3

import binding_generator
import sys

binding_generator.generate_bindings(sys.argv[1], "True", "64", "single", sys.argv[2])

