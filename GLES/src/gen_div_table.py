import math

FIXED_SHIFT = 16

def fixed(d):
    return math.ceil(d * (1 << FIXED_SHIFT))

MAX_DIV = 2048

print("GLfixed div_table[] = {")
print("0, ", end='')

for i in range(1, MAX_DIV):
    if (i + 1) % 16 != 0:
        end = ', '
    else:
        end = ',\n' if i != MAX_DIV - 1 else '\n'

    print(fixed(1 / i), end=end)

print("};")
