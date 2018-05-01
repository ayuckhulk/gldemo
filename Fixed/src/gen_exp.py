import math

FIXED_SHIFT = 16

def fixed(d):
    return math.ceil(d * (1 << FIXED_SHIFT))

print("static fixed ln_2 = {};".format(fixed(math.log(2))))

print("static fixed ln_table[] = {")

j = 0
for i in range(0x0000, 0x10000, 0x100):
    if j != 15:
        j += 1
        end = ', '
    else:
        j = 0
        end = ',\n' if i != 0x10000 - 0x100 else '\n'
    if i != 0:
        print(fixed(math.log(i / (1 << FIXED_SHIFT))), end=end)
    else:
        print("-1", end=end)

print("};")
print()
print("static fixed exp_table[] = {")

for i in range(0, 31):
    r = fixed(math.exp(math.pow(2, i - 16))) if i < 20 else 0

    if i == 15:
        end = ',\n'
    elif i == 30:
        end = '\n'
    else:
        end = ', '

    print(r, end=end)

print("};")
