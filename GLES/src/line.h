{
    int x0, x1, y0, y1;

    x0 = fixed_round_toint(v0->v[X]);
    x1 = fixed_round_toint(v1->v[X]);

    if (fixed_abs(v1->v[X] - v0->v[X]) > fixed_abs(v1->v[Y] - v0->v[Y])) {
        int dx = x1 > x0 ? 1 : -1;
        int n = x1 - x0 + 1;
        int x = x0;
        fixed dy = fixed_div(v1->v[Y] - v0->v[Y], v1->v[X] - v0->v[X]);
        fixed y = v0->v[Y];
        int iy;

        if (n < 0) n = -n;

        for (v = 0;v < MAX_VAR;v++) var_dv[v] = (var_v1[v] - var_v0[v]) / n;

        while (x <= x1) {
            iy = fixed_round_toint(y);
            framebuffer = (PIXEL *)((char *)context->framebuffer + context->pitch * iy);
            framebuffer += x;

            // fragmentcolor

            for (v = 0;v < MAX_VAR;v++) var_v[v] += var_dv[v];

            y += dy;
            x += dx;
        }
    } else {

    }
}
