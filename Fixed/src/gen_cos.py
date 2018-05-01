import math

FIXED_SHIFT = 16

def fixed(d):
    return math.ceil(d * (1 << FIXED_SHIFT))

COS_BITS = 12
COS_TABLE_SIZE = 1 << (COS_BITS - 1)

print("#define COS_BITS {}".format(COS_BITS))
print("#define ANGLE_CLAMP ((1 << COS_BITS) - 1)")
print("#define COS_TABLE_SIZE (1 << (COS_BITS - 1))")
print("#define DEGTOX fixed_double(((double) ANGLE_CLAMP / (double) 359))")
print()
print("static fixed cos_table[COS_TABLE_SIZE] = {")

j = 0
for i in range(COS_TABLE_SIZE):
    if j != 15:
        j += 1
        end = ', '
    else:
        j = 0
        end = ',\n' if i != COS_TABLE_SIZE - 1 else '\n'

    print(fixed(math.cos(i * math.pi / COS_TABLE_SIZE)), end=end)

print("};")
