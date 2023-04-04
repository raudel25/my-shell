import os


def parser(s: str) -> str:
    resp = ''
    act = False

    for i in s:
        if i == '\\':
            act = True
            continue

        if i == '\n':
            if act:
                resp += '\\n'
                act = False
            else:
                resp += ' '
            continue

        resp += i

    return resp


def to_char_array_c(name: str, files: str) -> str:
    s = 'char *'+name+f'[{len(files)}]'+' = { '
    for i in files:
        s += '"'
        s += i
        s += '",\n'

    s += '};\n'

    return s


files = []
content = []

for i in os.listdir('.'):
    if i.endswith('.txt'):
        files.append(i[0:-4])

for i in files:
    f = open(i+'.txt', "r")
    aux = parser(f.read())
    content.append(aux)
    f.close()

commands = to_char_array_c('commands', files)
commands_help = to_char_array_c('commands_help', content)

fc = open('../help.c', "w")
fc.write(commands+commands_help)
fc.close()