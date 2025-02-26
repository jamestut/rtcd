#!/usr/bin/env python3
import sys
import os
import subprocess
import multiprocessing
import itertools
import argparse

def printr(*args, **kwargs):
    kwargs['file'] = sys.stderr
    print(*args, **kwargs)

if __name__ == '__main__':
    # valid targets: 'clang --print-targets'
    TARGETS = ['aarch64', 'x86_64']
    TRIPLET_SUFFIX = 'redhat-linux-gnu'

    SYSROOT_NAME = 'libc'
    CFLAGS = ['-O2', '-g0']

    EXE_NAMES = ['rtcd', 'rtcctl']

    ap = argparse.ArgumentParser()
    ap.add_argument('--single', action='store_true',
        help="If specified, run the 'make' command single-threaded.")
    ap.add_argument('--verbose-linker', action='store_true',
        help="Pass '-v' option to the LDFLAGS.")
    ap.add_argument('targets', nargs='*',
        help=f'Specify which platforms to build. Supported platforms: {" ".join(TARGETS)}')
    args = ap.parse_args()

    if args.targets:
        if not all(i in TARGETS for i in args.targets):
            printr("One or more of the specified targets are not supported.")
            sys.exit(1)
        TARGETS = args.targets

    if 'CC' not in os.environ:
        printr('Please specify clang executable in $CC variable.')
        sys.exit(1)

    # check that $CC is actually clang
    sp = subprocess.run(
        [os.environ['CC'], '--version'],
        stdout=subprocess.PIPE,
        check=True
    )
    cc_version = sp.stdout.splitlines()[0]
    if b'clang' not in cc_version:
        printr('Ensure that $CC is clang.')
        sys.exit(1)

    # check that $LD llvm's linker
    ld_str = subprocess.run(
        ['ld', '-version'],
        stdout=subprocess.PIPE,
        check=True
    ).stdout
    if not (b'LLD' in ld_str and b'compatible with GNU linkers' in ld_str):
        printr('Ensure that system ld is lld.')
        sys.exit(1)

    # ensure the sysroots are specified
    SYSROOT_VARS=[f'SYSROOT_{i.replace('-', '_')}' for i in TARGETS]
    sysroot_specified = True
    for tgt, v in zip(TARGETS, SYSROOT_VARS):
        if v not in os.environ:
            sysroot_specified = False
            printr(f"Please specify sysroot for target '{tgt}' in ${v} variable.")
        else:
            if ' ' in os.environ[v]:
                printr("Ensure no space character in SYSROOT variables.")
                exit(1)
    if not sysroot_specified:
        exit(1)

    make_env = os.environ.copy()
    ldflags = ['--rtlib=compiler-rt']
    if args.verbose_linker:
        ldflags.append('-v')
    make_env['LDFLAGS'] = ' '.join(ldflags)

    nproc = 1 if args.single else multiprocessing.cpu_count()
    nproc = str(nproc)

    subprocess.run(['make', 'cleanartefacts'], check=True)

    for tgt, sysroot_var in zip(TARGETS, SYSROOT_VARS):
        print(f'Building for {tgt} ...')
        make_env['CFLAGS'] = f'--target={tgt}-{TRIPLET_SUFFIX} --sysroot {os.environ[sysroot_var]}'
        subprocess.run(['make', 'clean'], check=True)
        subprocess.run(['make', f'-j{nproc}'], env=make_env, check=True)
        for exename in EXE_NAMES:
            subprocess.run(['strip', exename], check=True)
        subprocess.run(itertools.chain(
            ['zip', f'rtcd-{SYSROOT_NAME}-{tgt}.zip'],
            EXE_NAMES
        ), check=True)

    print("Done!")
