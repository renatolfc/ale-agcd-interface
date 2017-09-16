#!/usr/bin/env python

import os
import sys

def get_last_line(fn):
    with open(fn) as fp:
        fp.seek(-2, os.SEEK_END)
        while fp.read(1) != b"\n":
            fp.seek(-2, os.SEEK_CUR)
        return fp.readline()

def make_structure(root):
    def mkdir(path):
        try:
            os.makedirs(path)
        except:
            pass
    mkdir(root)
    mkdir(os.path.join(root, 'trajectories'))
    mkdir(os.path.join(root, 'screens'))

def main(root, new_root):
    fns = [os.path.join(root, fn) for fn in os.listdir(root)
           if not fn.startswith('.')]
    scores = [int(get_last_line(fn).split(',')[2].strip()) for fn in fns]
    make_structure(new_root)
    for i, (_, fn) in enumerate(sorted(zip(scores, fns), reverse=True)):
        basename = os.path.basename(fn)
        dirname = os.path.dirname(os.path.dirname(fn))
        number = basename[:basename.find('.')]
        os.symlink(fn, os.path.join(new_root, 'trajectories', '%d.txt' % (i + 1)))
        os.symlink(os.path.join(dirname, 'screens', number),
                   os.path.join(new_root, 'screens', '%d' % (i + 1)))

if __name__ == '__main__':
    main(sys.argv[1], sys.argv[2])
