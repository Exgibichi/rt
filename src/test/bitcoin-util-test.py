#!/usr/bin/python
# Copyright 2014 BitPay, Inc.
# Distributed under the GPL3 software license, see the accompanying
# file COPYING or http://www.gnu.org/licenses/gpl.html.

import os
import bctest
import buildenv

if __name__ == '__main__':
	bctest.bctester(os.environ["srcdir"] + "/test/data",
			"bitcoin-util-test.json",buildenv)

