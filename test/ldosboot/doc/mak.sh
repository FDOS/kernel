#! /bin/bash

# Usage of the works is permitted provided that this
# instrument is retained with the works, so that any entity
# that uses the works is notified of this instrument.
#
# DISCLAIMER: THE WORKS ARE WITHOUT WARRANTY.

echo -ne "\\U Source Control Revision ID\n\nhg $(hg id -i), from commit on at $(hg log -r . --template="{date|isodatesec}\n")\n\nIf this is in ecm's repository, you can find it at \\W{https://hg.pushbx.org/ecm/ldosboot/rev/$(hg log -r . --template "{node|short}")}{https://hg.pushbx.org/ecm/ldosboot/rev/$(hg log -r . --template "{node|short}")}\n" > screvid.src
halibut --precise ldosboot.src screvid.src --html --text --pdf
