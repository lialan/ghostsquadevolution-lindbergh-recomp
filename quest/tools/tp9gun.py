#!/usr/bin/env python3
"""tp9gun.py <out.h> - a B&T TP9 for the Quest overlay (xr_overlay.c), built here.

No free-licensed TP9 model exists, so this one is made from side profiles,
each extruded across the gun, sized from the real pistol (about 303 mm long,
45 mm wide, 130 mm barrel). Same gun space as obj2gun.py: metres, x across,
y up, barrel along -Z on the aim ray (the bore at y = 0), the pistol grip
where the SMG's is (85 mm behind the aim pose). Each part is a convex
polygon in (z, y) and a half width; the output is triangles as position,
normal, sRGB colour (9 floats a vertex).
"""
import math, sys

BLACK = (0.24, 0.24, 0.25)      # the polymer frame
DARK = (0.18, 0.18, 0.19)       # grips, magazine
STEEL = (0.32, 0.32, 0.33)      # rail, barrel, bolt

# (name, colour, half width, [(z, y) ...] convex, either winding)
PARTS = [
    # upper receiver: flat top, the front stepped down into the barrel shroud
    ('receiver', BLACK, 0.0215, [(-0.150, 0.030), (-0.160, 0.016), (-0.160, -0.026), (-0.148, -0.034),
                                 (0.138, -0.034), (0.150, -0.022), (0.150, 0.030)]),
    ('shroud', BLACK, 0.0150, [(-0.178, 0.014), (-0.178, -0.016), (-0.160, -0.022), (-0.160, 0.018)]),
    # the top rail, full length, with the cross slots as raised ribs
    ('rail', STEEL, 0.0110, [(-0.156, 0.030), (-0.156, 0.036), (0.148, 0.036), (0.148, 0.030)]),
    # the stock mount block and charging handle at the back
    ('mount', DARK, 0.0170, [(0.150, 0.022), (0.150, -0.020), (0.164, -0.016), (0.164, 0.018)]),
    ('handle', STEEL, 0.0260, [(0.150, 0.034), (0.150, 0.024), (0.160, 0.024), (0.160, 0.034)]),
    # the lower: pistol grip, raked back, the magazine well inside it
    ('grip', DARK, 0.0165, [(0.062, -0.034), (0.112, -0.034), (0.122, -0.060), (0.130, -0.128),
                            (0.084, -0.128), (0.068, -0.060)]),
    ('beavertail', BLACK, 0.0175, [(0.108, -0.030), (0.140, -0.034), (0.124, -0.050), (0.112, -0.046)]),
    # the trigger guard, a closed loop whose front is the hand stop
    ('guard_front', BLACK, 0.0140, [(-0.014, -0.034), (0.006, -0.034), (0.004, -0.096), (-0.020, -0.100),
                                    (-0.026, -0.090)]),
    ('guard_bottom', BLACK, 0.0090, [(-0.010, -0.090), (-0.008, -0.100), (0.074, -0.100), (0.072, -0.090)]),
    ('trigger', STEEL, 0.0035, [(0.038, -0.034), (0.046, -0.034), (0.044, -0.054), (0.036, -0.066),
                                (0.032, -0.064), (0.036, -0.052)]),
    # a 30-round magazine out of the bottom of the grip
    ('magazine', DARK, 0.0125, [(0.090, -0.126), (0.120, -0.126), (0.124, -0.232), (0.092, -0.236)]),
    ('mag_base', BLACK, 0.0145, [(0.088, -0.232), (0.126, -0.228), (0.126, -0.242), (0.088, -0.246)]),
]
for i in range(12):             # rail slots
    z = -0.148 + i * 0.0246
    PARTS.append(('rib', STEEL, 0.0115, [(z, 0.036), (z, 0.039), (z + 0.016, 0.039), (z + 0.016, 0.036)]))
RAIL_Y = 0.039

def prism(colour, hw, poly):
    """A convex (z, y) polygon extruded from x = -hw to +hw."""
    # wind counter-clockwise seen from +x (z to the right is -z forward... just make area positive)
    a = sum(poly[i][0] * poly[(i + 1) % len(poly)][1] - poly[(i + 1) % len(poly)][0] * poly[i][1]
            for i in range(len(poly)))
    if a < 0: poly = poly[::-1]
    tris = []
    def tri(p, q, r):
        u = [q[i] - p[i] for i in range(3)]; w = [r[i] - p[i] for i in range(3)]
        n = [u[1] * w[2] - u[2] * w[1], u[2] * w[0] - u[0] * w[2], u[0] * w[1] - u[1] * w[0]]
        l = math.sqrt(sum(x * x for x in n)) or 1.0
        n = [x / l for x in n]
        tris.append([(p, n, colour), (q, n, colour), (r, n, colour)])
    pt = lambda x, zy: (x, zy[1], zy[0])
    m = len(poly)
    for k in range(1, m - 1):    # the two caps
        tri(pt(hw, poly[0]), pt(hw, poly[k + 1]), pt(hw, poly[k]))
        tri(pt(-hw, poly[0]), pt(-hw, poly[k]), pt(-hw, poly[k + 1]))
    for k in range(m):           # the sides
        p0, p1 = poly[k], poly[(k + 1) % m]
        tri(pt(-hw, p0), pt(-hw, p1), pt(hw, p1))
        tri(pt(-hw, p0), pt(hw, p1), pt(hw, p0))
    return tris

def barrel(z0, z1, r, colour, sides=10):
    tris = []
    for k in range(sides):
        a0, a1 = 2 * math.pi * k / sides, 2 * math.pi * (k + 1) / sides
        p = [(r * math.cos(a), r * math.sin(a)) for a in (a0, a1)]
        n = [(math.cos(a), math.sin(a), 0.0) for a in (a0, a1)]
        q00, q01 = (p[0][0], p[0][1], z0), (p[1][0], p[1][1], z0)
        q10, q11 = (p[0][0], p[0][1], z1), (p[1][0], p[1][1], z1)
        tris.append([(q00, n[0], colour), (q01, n[1], colour), (q11, n[1], colour)])
        tris.append([(q00, n[0], colour), (q11, n[1], colour), (q10, n[0], colour)])
        cap = (0.0, 0.0, -1.0)
        tris.append([((0.0, 0.0, z0), cap, colour), (q01, cap, colour), (q00, cap, colour)])
    return tris

def main(out):
    tris = []
    for _, col, hw, poly in PARTS:
        tris += prism(col, hw, poly)
    tris += barrel(-0.206, -0.176, 0.0085, STEEL)     # the threaded muzzle past the shroud
    with open(out, 'w') as f:
        f.write('/* Generated by quest/tools/tp9gun.py: a low-poly B&T TP9, built from its\n'
                ' * published dimensions. Gun space, metres: barrel along -Z on the aim ray;\n'
                ' * position, normal, sRGB colour. */\n')
        f.write('#define TP9_MUZZLE_Z -0.2060f\n#define TP9_RAIL_Y %.4ff\n' % RAIL_Y)
        f.write('#define TP9_VERTS %d\nstatic const float TP9_MESH[TP9_VERTS * 9] = {\n' % (len(tris) * 3))
        for t in tris:
            for p, n, c in t:
                f.write('    %.4ff, %.4ff, %.4ff, %.3ff, %.3ff, %.3ff, %.3ff, %.3ff, %.3ff,\n' % (*p, *n, *c))
        f.write('};\n')
    print(f'{len(tris)} triangles')

main(sys.argv[1])
