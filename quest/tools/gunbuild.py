#!/usr/bin/env python3
"""gunbuild.py <tp9|ghostsquad> <out.h> - a gun for the Quest overlay (xr_overlay.c),
built here from side profiles, each extruded across the gun.

tp9: a B&T TP9, sized from the real pistol (about 303 mm long, 45 mm wide,
130 mm barrel); no free-licensed model of it exists.
ghostsquad: the Ghost Squad cabinet's gun controller, traced from photos of
the cabinet (IMFDB, "Ghost Squad": the arcade version's controller) and
scaled to about 72 cm.

Same gun space as obj2gun.py: metres, x across,
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
TP9 = [
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
    TP9.append(('rib', STEEL, 0.0115, [(z, 0.036), (z, 0.039), (z + 0.016, 0.039), (z + 0.016, 0.036)]))

# The cabinet's gun, in pixels of the right-side photo (854x480, muzzle to the
# right): x along the gun, y down. px() turns them into gun space.
PX_M, PX_GRIP, PX_BORE = 0.00104, 380, 185     # metres a pixel; the grip's x; the bore's y
def px(pts):
    return [(0.085 - (x - PX_GRIP) * PX_M, -(y - PX_BORE) * PX_M) for x, y in pts]
LENS = (0.70, 0.74, 0.78)
GS = [
    ('receiver', BLACK, 0.0220, px([(330, 145), (560, 145), (565, 160), (565, 222), (330, 226)])),
    ('foreend', BLACK, 0.0250, px([(555, 150), (728, 150), (738, 160), (738, 205), (728, 212), (555, 212)])),
    ('top_bar', BLACK, 0.0120, px([(340, 124), (700, 124), (700, 138), (340, 138)])),
    ('rear_post', BLACK, 0.0120, px([(340, 124), (362, 124), (368, 146), (336, 146)])),
    ('front_ear', BLACK, 0.0140, px([(640, 150), (668, 112), (702, 112), (702, 150)])),
    ('magwell', BLACK, 0.0170, px([(492, 220), (552, 220), (552, 246), (492, 246)])),
    ('magazine', DARK, 0.0120, px([(500, 240), (546, 240), (556, 332), (514, 336)])),
    ('grip', DARK, 0.0170, px([(360, 222), (432, 222), (420, 260), (390, 336), (324, 330), (348, 262)])),
    ('trigger_house', BLACK, 0.0175, px([(345, 222), (500, 222), (500, 240), (345, 240)])),
    ('guard', BLACK, 0.0100, px([(395, 258), (490, 252), (492, 266), (397, 270)])),
    ('guard_front', BLACK, 0.0100, px([(478, 238), (494, 238), (494, 266), (478, 266)])),
    ('trigger', STEEL, 0.0035, px([(410, 236), (420, 236), (416, 256), (406, 252)])),
    ('foregrip', DARK, 0.0160, px([(598, 210), (642, 210), (638, 318), (604, 320)])),
    # the fixed stock: a top bar and a lower bar to the butt, a thin web between
    ('stock_top', BLACK, 0.0200, px([(40, 138), (334, 140), (334, 166), (40, 166)])),
    ('stock_bottom', BLACK, 0.0200, px([(60, 228), (334, 200), (334, 226), (60, 256)])),
    ('stock_web', DARK, 0.0100, px([(60, 160), (334, 160), (334, 206), (60, 236)])),
    ('butt', BLACK, 0.0220, px([(18, 134), (62, 134), (62, 258), (18, 254)])),
    ('cocking', STEEL, 0.0300, px([(600, 140), (618, 140), (618, 150), (600, 150)])),
]
for i in range(6):              # the fore-end's row of vents
    x = 575 + i * 24
    GS.append(('vent', DARK, 0.0255, px([(x, 165), (x + 12, 165), (x + 12, 177), (x, 177)])))

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

def barrel(z0, z1, r, colour, sides=10, y=0.0):
    tris = []
    for k in range(sides):
        a0, a1 = 2 * math.pi * k / sides, 2 * math.pi * (k + 1) / sides
        p = [(r * math.cos(a), y + r * math.sin(a)) for a in (a0, a1)]
        n = [(math.cos(a), math.sin(a), 0.0) for a in (a0, a1)]
        q00, q01 = (p[0][0], p[0][1], z0), (p[1][0], p[1][1], z0)
        q10, q11 = (p[0][0], p[0][1], z1), (p[1][0], p[1][1], z1)
        tris.append([(q00, n[0], colour), (q01, n[1], colour), (q11, n[1], colour)])
        tris.append([(q00, n[0], colour), (q11, n[1], colour), (q10, n[0], colour)])
        cap = (0.0, 0.0, -1.0)
        tris.append([((0.0, y, z0), cap, colour), (q01, cap, colour), (q00, cap, colour)])
    return tris

def main(which, out):
    tris = []
    if which == 'tp9':
        name, parts, rail = 'TP9', TP9, 0.039
        tris += barrel(-0.206, -0.176, 0.0085, STEEL)     # the threaded muzzle past the shroud
        muzzle, what = -0.206, 'a low-poly B&T TP9, built from its\n * published dimensions'
    else:
        name, parts, rail = 'GS', GS, px([(0, 124)])[0][1]
        (z0, _), (z1, _) = px([(760, 0), (734, 0)])
        tris += barrel(z0, z1, 15 * PX_M, STEEL)          # the muzzle
        (l0, ly), (l1, _) = px([(764, 212), (700, 0)])
        tris += barrel(l0 + 0.004, l1, 18 * PX_M, DARK, y=ly)   # the flashlight under it
        tris += barrel(l0, l0 + 0.004, 15 * PX_M, LENS, y=ly)   # its lens
        muzzle, what = z0, "the Ghost Squad cabinet's gun, traced from\n * photos of the cabinet"
    for _, col, hw, poly in parts:
        tris += prism(col, hw, poly)
    with open(out, 'w') as f:
        f.write('/* Generated by quest/tools/gunbuild.py: %s. Gun space, metres: barrel\n'
                ' * along -Z on the aim ray; position, normal, sRGB colour. */\n' % what)
        f.write('#define %s_MUZZLE_Z %.4ff\n#define %s_RAIL_Y %.4ff\n' % (name, muzzle, name, rail))
        f.write('#define %s_VERTS %d\nstatic const float %s_MESH[%s_VERTS * 9] = {\n' % (name, len(tris) * 3, name, name))
        for t in tris:
            for p, n, c in t:
                f.write('    %.4ff, %.4ff, %.4ff, %.3ff, %.3ff, %.3ff, %.3ff, %.3ff, %.3ff,\n' % (*p, *n, *c))
        f.write('};\n')
    print(f'{len(tris)} triangles')

main(*sys.argv[1:3])
