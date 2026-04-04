# Collision Detection & Resolution

Two collision scenarios: **car vs curve** (track walls) and **car vs car**.

## Data Structures

`CollisionResult` carries everything the resolver needs:

| Field          | Meaning                                         |
|----------------|--------------------------------------------------|
| `collided`     | whether a collision occurred                     |
| `normal`       | surface normal pointing *away* from the obstacle |
| `penetration`  | how far the car sank into the obstacle           |
| `contactPoint` | world-space point of deepest penetration         |

Cars expose an **oriented bounding box** (`RectangleOBB`) — 4 corners + 2 edge-normal axes. Track walls are `ParametricCurve2D` objects sampled into polylines.

---

## Car vs Curve (track wall)

### Detection

The car's OBB is tested against the nearest point on the curve's polyline. Eight sample points are used (4 corners + 4 edge midpoints).

For each sample point **p**:

1. Find the closest point **c** on the curve and the outward unit normal **n** at that location.
2. Compute the signed distance: \
   `d = (p − c) · n`
3. Whether `d` means "penetrating" depends on which side of the curve the car centre sits (inner vs outer boundary). The deepest penetrating point wins.

### Resolution

1. **Position correction** — push the car out along the collision normal:

   \[
   \mathbf{x'} = \mathbf{x} + \mathbf{n} \cdot (penetration + \varepsilon)
   \]

   where ε is a small skin width to prevent surface oscillation.

2. **Speed correction** — if the car is moving into the wall, reduce its speed along its forward axis:

   \[
   s' = s \cdot \bigl(1 - (1 + e)\cos^2\alpha\bigr)
   \]

   where *e* = wall restitution (0.3), *α* = angle between the car's forward direction and the wall normal, and *s* is the car's scalar speed. A head-on hit (cos α ≈ 1) kills most speed; a glancing hit (cos α ≈ 0) barely slows the car.

---

## Car vs Car (OBB–OBB)

### Detection — Separating Axis Theorem

Two convex shapes overlap **iff** their projections overlap on every candidate separating axis. For two OBBs this is the 4 edge normals (2 per box).

For each axis **a**:

1. Project all 8 corners onto **a**: `proj = corner · a`
2. Find the projection intervals `[min₁, max₁]` and `[min₂, max₂]`.
3. Compute overlap: `overlap = min(max₁, max₂) − max(min₁, min₂)`
4. If `overlap ≤ 0` → no collision (separating axis found, early exit).

The axis with the **smallest positive overlap** gives the collision normal and penetration depth. The normal is oriented to point from car 1 toward car 2.

### Resolution — 1-D Impulse (equal mass)

1. **Position correction** — push both cars apart equally:

   \[
   \mathbf{x_1'} = \mathbf{x_1} - \tfrac{1}{2}\mathbf{n}(penetration + \varepsilon), \quad
   \mathbf{x_2'} = \mathbf{x_2} + \tfrac{1}{2}\mathbf{n}(penetration + \varepsilon)
   \]

2. **Impulse** — compute the relative velocity along the normal, then apply a 1-D impulse assuming equal unit masses:

   \[
   v_{rel} = (\mathbf{v_1} - \mathbf{v_2}) \cdot \mathbf{n}
   \]
   \[
   j = -\frac{(1 + e)\, v_{rel}}{2}
   \]

   where *e* = car restitution (0.7). Each car's velocity is adjusted by ±**n**·*j*, then projected back onto its own forward axis to get the new scalar speed.

---

## Constants

| Constant           | Value | Purpose                                    |
|--------------------|-------|--------------------------------------------|
| `WALL_RESTITUTION` | 0.3   | Bounciness of wall hits (low = absorptive) |
| `CAR_RESTITUTION`  | 0.7   | Bounciness of car-car hits                 |
| `SKIN_WIDTH`       | 0.01  | Positional buffer to prevent jitter        |
