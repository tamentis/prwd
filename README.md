# prwd - universal shell prompt

prwd is a replacement for your shell's PS1, it provides a simple templating
language with commands to customize your prompt.  prwd keeps your prompt
consistent between shells, provides fast template commands to get your
hostname, git or mercurial branch without an inordinate amount of slow shell
scripting.

## Setup
Add the following line in your shell's config file (e.g. `.profile` or `.bashrc`):

    export PS1='`prwd`'

## Template syntax
All the commands start with `${` and end with `}`, anything else is considered
static.  Commands within the brackets have flags similarly to standard UNIX
tools.  Here is an example template definition:

    ${date} ${hostname}:${branch}:${path -l 24}${uid}

Which would render as:

    18:29:22 prometheus:master:/tmp$

Your prompt template can be defined in three different places:
 1. using a PRWD environment variable
 2. using the "template" keyword in your ~/.prwdrc file
 3. using the -t parameter to prwd

## Template commands
Check the `prwdrc` man page for detailed command information.  The following
commands are available:
 - `path`
 - `branch`
 - `date`
 - `hostname`
 - `uid`
 - `color`
 - `sep`

## Aliases
If you use the `path` command in your prompt template, defining aliases will
allow path to shorten your current path using your own defined keywords.  For
example, add the following to your `prwdrc` file:

    alias *prwd /home/tamentis/projects/prwd

And navigating to `/home/tamentis/projects/prwd/doc` will allow prwd to reduce
your current path to `*prwd/doc`.  Another good example is that `~` is a
default alias for prwd, you can create more of these ultra-short aliases if you
want.

## Project root finder

This tool also comes with a project root finder. You can start it simply with
`prwd -f`. It will print the closest project root, this is particularly helpful
with a shell function added to your profile such as:

    # cdr - CD to the nearest folder considered a root
    cdr() {
        if path=`prwd -f`; then
            cd $path
        else
            echo "error: couldn't not find target"
            return 1
        fi
    }

A root is defined as the first folder with a .git or .hg folder, if none is
found, the first folder with a README file. If you want to setup an alias with
a custom file, you can also use `prwd -F filename`.

## Installation

    ./configure
    make
    sudo make install

## Hacking on prwd
 - Start from OpenBSD's style(9) man page.
 - Use parenthesis with your returns.
 - Use brackets even for one-line blocks or at least add a new line after.
 - Unless a value is only used as boolean, do not use '!', explicitely check
   for the value (e.g. !strcmp to test equality is not explicit).
 - No malloc/free, all the memory should be automatically allocated in prwd.
