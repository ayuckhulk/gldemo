{
    GLfixed x0, x1, x2;
    GLint y0, y1, y2;

    GLfixed dxdy01, dxdy02, dxdy12;
    GLfixed x_left, dxdy_left;
    GLfixed x_right, dxdy_right;

#ifdef R_PERSPECTIVE
    GLfixed p_1w_0, p_1w_1, p_1w_2;
    GLfixed p_1w_d01, p_1w_d02, p_1w_d12;
    GLfixed p_1w_left, p_1w_dleft;
    GLfixed p_1w_right, p_1w_dright;
    GLfixed p_w_left, p_w_right, p_w_mid;

    GLfixed var_ddv[MAX_VAR];
#endif
    GLfixed var_v[MAX_VAR];
    GLfixed var_dv[MAX_VAR];
    GLfixed var_v0[MAX_VAR];
    GLfixed var_v1[MAX_VAR];
    GLfixed var_v2[MAX_VAR];
    GLfixed var_d01[MAX_VAR];
    GLfixed var_d02[MAX_VAR];
    GLfixed var_d12[MAX_VAR];
    GLfixed var_left[MAX_VAR];
    GLfixed var_dleft[MAX_VAR];
    GLfixed var_right[MAX_VAR];
    GLfixed var_dright[MAX_VAR];

#ifdef R_LOD
    GLint lod;
#endif

#if defined(R_FLAT_COLOR)
    GLubyte *c = context->flat_color;
    PIXEL color = RGB_TO_PIXEL(c[0], c[1], c[2]);
#ifdef R_BLEND
    GLubyte alpha = toAlpha(c[3]);
#endif
#endif

    Vertex *t;
    GLboolean update_left;

#ifdef R_TEXTURE0
    Texture *texture0;
    register PIXEL *t0_texels;
    register GLubyte *t0_alpha;
    register GLint t0_offset;
    GLint t0_s_mask;
    GLint t0_t_mask;
    GLint t0_t_shift;
#endif

#ifdef R_ALPHA_TEST
    register GLuint alpha_value = context->alpha_value;
#endif

    GLint i, v;

    if (v1->v.xyzw[Y] < v0->v.xyzw[Y]) {
        t = v0;
        v0 = v1;
        v1 = t;
    }
    if (v2->v.xyzw[Y] < v0->v.xyzw[Y]) {
        t = v2;
        v2 = v1;
        v1 = v0;
        v0 = t;
    } else if (v2->v.xyzw[Y] < v1->v.xyzw[Y]) {
        t = v1;
        v1 = v2;
        v2 = t;
    }

    x0 = fixed_toint(v0->v.xyzw[X]);
    x1 = fixed_toint(v1->v.xyzw[X]);
    y0 = fixed_toint(v0->v.xyzw[Y]);
    y1 = fixed_toint(v1->v.xyzw[Y]);

    if ((y0 == y1) && (x1 < x0)) {
        t = v0;
        v0 = v1;
        v1 = t;
    }

    x0 = fixed_round(v0->v.xyzw[X]);
    x1 = fixed_round(v1->v.xyzw[X]);
    x2 = fixed_round(v2->v.xyzw[X]);

    y0 = fixed_round(v0->v.xyzw[Y]);
    y1 = fixed_round(v1->v.xyzw[Y]);
    y2 = fixed_round(v2->v.xyzw[Y]);

#ifndef R_TEXTURE0_REPLACE
    if (context->shade_model == GL_FLAT) {

    	v0->color[G] = context->flat_color[1];
    	v1->color[G] = context->flat_color[1];
    	v2->color[G] = context->flat_color[1];

#ifndef R_GREY_COLOR
    	v0->color[R] = context->flat_color[0];
    	v0->color[B] = context->flat_color[2];
    	v0->color[A] = context->flat_color[3];
    	v1->color[R] = context->flat_color[0];
    	v1->color[B] = context->flat_color[2];
    	v1->color[A] = context->flat_color[3];
    	v2->color[R] = context->flat_color[0];
    	v2->color[G] = context->flat_color[1];
    	v2->color[B] = context->flat_color[2];
    	v2->color[A] = context->flat_color[3];
#endif
    }
#endif

    var_v0[VZ] = v0->v.xyzw[Z];
    var_v1[VZ] = v1->v.xyzw[Z];
    var_v2[VZ] = v2->v.xyzw[Z];

#ifdef R_TEXTURE0
    var_v0[VS] = v0->t[0].p.x;
    var_v0[VT] = v0->t[0].p.y;
    var_v1[VS] = v1->t[0].p.x;
    var_v1[VT] = v1->t[0].p.y;
    var_v2[VS] = v2->t[0].p.x;
    var_v2[VT] = v2->t[0].p.y;
#endif

#ifndef R_TEXTURE0_REPLACE

    var_v0[VG] = fixed_int(v0->color[G]);
    var_v1[VG] = fixed_int(v1->color[G]);
    var_v2[VG] = fixed_int(v2->color[G]);

#ifndef R_GREY_COLOR
    var_v0[VR] = fixed_int(v0->color[R]);
    var_v0[VB] = fixed_int(v0->color[B]);


    var_v1[VR] = fixed_int(v1->color[R]);
    var_v1[VB] = fixed_int(v1->color[B]);


    var_v2[VR] = fixed_int(v2->color[R]);
    var_v2[VB] = fixed_int(v2->color[B]);
#endif

#if defined(R_BLEND) || defined(R_ALPHA_TEST)
    var_v0[VA] = fixed_int(v0->color[A]);
    var_v1[VA] = fixed_int(v1->color[A]);
    var_v2[VA] = fixed_int(v2->color[A]);
#endif
#endif

#ifdef R_PERSPECTIVE
//    p_1w_0 = fixed_div(fixed_int(1), (v0->v[W]));
//    p_1w_1 = fixed_div(fixed_int(1), (v1->v[W]));
//    p_1w_2 = fixed_div(fixed_int(1), (v2->v[W]));
//
//    //printf("w0=%f w1=%f w2=%f\n", fixed_tofloat(v0->v[W]), fixed_tofloat(v1->v[W]), fixed_tofloat(v2->v[W]));
//
//    for (v = MAX_LINEAR_VAR;v < MAX_VAR;v++) {
//        var_v0[v] = fixed_mul(var_v0[v], p_1w_0);
//        var_v1[v] = fixed_mul(var_v1[v], p_1w_1);
//        var_v2[v] = fixed_mul(var_v2[v], p_1w_2);
//    }

    {
        GLfixed w0, w1, w2;
        w0 = v0->v.xyzw[W] << 0;
        w1 = v1->v.xyzw[W] << 0;
        w2 = v2->v.xyzw[W] << 0;
    for (v = MAX_LINEAR_VAR;v < MAX_VAR;v++) {
        var_v0[v] = fixed_div(var_v0[v], w0);
        var_v1[v] = fixed_div(var_v1[v], w1);
        var_v2[v] = fixed_div(var_v2[v], w2);
    }

    p_1w_0 = fixed_div(fixed_int(1) << R_W_SHIFT, w0);
    p_1w_1 = fixed_div(fixed_int(1) << R_W_SHIFT, w1);
    p_1w_2 = fixed_div(fixed_int(1) << R_W_SHIFT, w2);
    }
#endif

#ifdef R_TEXTURE0
    texture0 = context->TU[0]->texture;
#ifndef R_LOD
    t0_texels = texture0->lod[0].texels;
    t0_alpha = texture0->lod[0].alpha;
    t0_s_mask = texture0->lod[0].s_mask;
    t0_t_mask = texture0->lod[0].t_mask;
    t0_t_shift = texture0->lod[0].t_shift;
#endif
#endif

    y0 = fixed_toint(y0);
    y1 = fixed_toint(y1);
    y2 = fixed_toint(y2);

    if (y1 != y0) {
        GLfixed one_over_dy01 = div_table[y1 - y0];

        dxdy01 = fixed_mul(x1 - x0, one_over_dy01);

#ifdef R_PERSPECTIVE
        p_1w_d01 = fixed_mul(p_1w_1 - p_1w_0, one_over_dy01);
#endif
        for (v = 0;v < MAX_VAR;v++) var_d01[v] = fixed_mul(var_v1[v] - var_v0[v], one_over_dy01);

    } else {
    	dxdy01 = 0;

#ifdef R_PERSPECTIVE
    	p_1w_d01 = 0;
#endif
    	for (v = 0;v < MAX_VAR;v++) var_d01[v] = 0;
    }

    if (y2 != y0) {
    	GLfixed one_over_dy02 = div_table[y2 - y0];

        dxdy02 = fixed_mul(x2 - x0, one_over_dy02);

#ifdef R_PERSPECTIVE
        p_1w_d02 = fixed_mul(p_1w_2 - p_1w_0, one_over_dy02);
#endif

        for (v = 0;v < MAX_VAR;v++) var_d02[v] = fixed_mul(var_v2[v] - var_v0[v], one_over_dy02);

    } else {
    	dxdy02 = 0;

#ifdef R_PERSPECTIVE
    	p_1w_d02 = 0;
#endif

    	for (v = 0;v < MAX_VAR;v++) var_d02[v] = 0;
    }

    if (y2 != y1) {
    	GLfixed one_over_dy12 = div_table[y2 - y1];

        dxdy12 = fixed_mul(x2 - x1, one_over_dy12);

#ifdef R_PERSPECTIVE
        p_1w_d12 = fixed_mul(p_1w_2 - p_1w_1, one_over_dy12);
#endif
        for (v = 0;v < MAX_VAR;v++) var_d12[v] = fixed_mul(var_v2[v] - var_v1[v], one_over_dy12);

    } else {
    	dxdy12 = 0;

#ifdef R_PERSPECTIVE
    	p_1w_d12 = 0;
#endif

    	for (v = 0;v < MAX_VAR;v++) var_d12[v] = 0;
    }

    x_left = x0;

#ifdef R_PERSPECTIVE
    p_1w_left = p_1w_0;
#endif
    for (v = 0;v < MAX_VAR;v++) var_left[v] = var_v0[v];

    if (y0 != y1) {
        x_right = x_left;

#ifdef R_PERSPECTIVE
        p_1w_right = p_1w_left;
#endif
        for (v = 0;v < MAX_VAR;v++) var_right[v] = var_left[v];

        update_left = dxdy02 > dxdy01;

        if (update_left) {
            dxdy_left = dxdy01;
            dxdy_right = dxdy02;

#ifdef R_PERSPECTIVE
            p_1w_dleft = p_1w_d01;
            p_1w_dright = p_1w_d02;
#endif
            for (v = 0;v < MAX_VAR;v++) {
                var_dleft[v] = var_d01[v];
                var_dright[v] = var_d02[v];
            }

        } else {
            dxdy_left = dxdy02;
            dxdy_right = dxdy01;

#ifdef R_PERSPECTIVE
            p_1w_dleft = p_1w_d02;
            p_1w_dright = p_1w_d01;
#endif
            for (v = 0;v < MAX_VAR;v++) {
                var_dleft[v] = var_d02[v];
                var_dright[v] = var_d01[v];
            }
        }
    } else {
        update_left = 0;

        x_right = x1;
        dxdy_left = dxdy02;

        dxdy_right = dxdy12;

#ifdef R_PERSPECTIVE
        p_1w_right = p_1w_1;
        p_1w_dleft = p_1w_d02;
        p_1w_dright = p_1w_d12;
#endif
        for (v = 0;v < MAX_VAR;v++) {
            var_right[v] = var_v1[v];
            var_dleft[v] = var_d02[v];
            var_dright[v] = var_d12[v];
        }

    } // y0 != y1

    if (y2 > context->ymax) y2 = context->ymax;

    for (i = y0; i <= y2; i++){

#ifdef R_BLEND
        register GLubyte fragment_alpha;
        register GLuint dest_color;
#endif

        register GLuint fragment_color;
        register GLint n;

        register PIXEL *framebuffer;
        register GLfixed *zbuffer;

        if (i == y1) {
            if (update_left) {
                dxdy_left = dxdy12;
                x_left = x1;

#ifdef R_PERSPECTIVE
                p_1w_left = p_1w_1;
                p_1w_dleft = p_1w_d12;
#endif
                for (v = 0;v < MAX_VAR;v++) {
                    var_dleft[v] = var_d12[v];
                    var_left[v] = var_v1[v];
                }

            } else {
                dxdy_right = dxdy12;
                x_right = x1;

#ifdef R_PERSPECTIVE
                p_1w_right = p_1w_1;
                p_1w_dright = p_1w_d12;
#endif
                for (v = 0;v < MAX_VAR;v++) {
                    var_dright[v] = var_d12[v];
                    var_right[v] = var_v1[v];
                }

            } // update_left
        } // i == y1

        if (i >= context->ymin) {
            GLint dx;
            GLfixed one_over_n;

            n = fixed_toint_round(x_right) - fixed_toint_round(x_left);

            dx = context->xmin - fixed_toint_round(x_left);

            if (n > 0 && dx < n && fixed_toint_round(x_left) <= context->xmax) {
				#ifdef R_PERSPECTIVE
                GLfixed n1, n2;
                #endif
                framebuffer = (PIXEL *)((char *)context->framebuffer + context->pitch * i);
                if (dx > 0) framebuffer += context->xmin;
                else framebuffer += fixed_toint_round(x_left);

                zbuffer = context->zbuffer + context->width * i;
                if (dx > 0) zbuffer += context->xmin;
                else zbuffer += fixed_toint_round(x_left);

                n++;

                one_over_n = div_table[n];

#ifdef R_PERSPECTIVE

                if (p_1w_left == 0) return;
                if (p_1w_right == 0) return;
                if ((p_1w_left + p_1w_right) == 0) return;

                //if (!(p_1w_left & ((1 << 22) - 1))) p_w = context->div_table[p_1w_left >> 6];
                //else {
                //p_w = fixed_div(fixed_int(1), p_1w_left);
                //}

                p_w_left = fixed_div(fixed_int(1) << R_W_SHIFT, p_1w_left);
                p_w_right = fixed_div(fixed_int(1) << R_W_SHIFT, p_1w_right);
                p_w_mid = fixed_div(fixed_int(1) << R_W_SHIFT, (p_1w_left + p_1w_right));
                n1 = fixed_mul(fixed_int(1) << R_W_SHIFT, one_over_n);
                //n2 = fixed_div(fixed_int(1) << R_W_SHIFT, fixed_int(n * n));
                n2 = fixed_mul(n1, one_over_n);
                /*
                f(n) = an^2+bn+c

                f(0) = var_left
                f(n) = var_right
                f(n/2) = (var_left + var_right) / (p_1w_left + p_1w_right)
                 */

                for (v = MAX_LINEAR_VAR;v < MAX_VAR;v++) {
                	GLfixed vl, vr, vm;
                	GLfixed a, b;

                	vl = fixed_mul(var_left[v], p_w_left);
                	vr = fixed_mul(var_right[v], p_w_right);
                	vm = fixed_mul((var_left[v] + var_right[v]), p_w_mid);

                	a = fixed_mul((((vl + vr) << 1) - (vm << 2)), n2);
                	b = fixed_mul((((vm - vl) << 2) - vr + vl), n1);


                	var_v[v] = vl;
                	var_dv[v] = (a + b) >> R_W_SHIFT;
                	var_ddv[v] = a >> (R_W_SHIFT - 1);

                	if (dx > 0) {
                		var_v[v] += (dx * (a >> R_W_SHIFT) + (b >> R_W_SHIFT)) * dx;
                		var_dv[v] += var_ddv[v] * dx;
                	}
                }

#if MAX_LINEAR_VAR > 0
                for (v = 0;v < MAX_LINEAR_VAR;v++) {
                    var_v[v] = var_left[v];
                    var_dv[v] = fixed_mul(var_right[v] - var_left[v], one_over_n);
                }
                if (dx > 0)
                    for (v = 0;v < MAX_LINEAR_VAR;v++) var_v[v] += var_dv[v] * dx;
#endif


#else
                for (v = 0;v < MAX_VAR;v++) {
                    var_v[v] = var_left[v];
                    var_dv[v] = fixed_mul(var_right[v] - var_left[v], one_over_n);
                }
                if (dx > 0)
                    for (v = 0;v < MAX_VAR;v++) var_v[v] += var_dv[v] * dx;
#endif

#ifdef R_LOD
    lod = fixed_toint(LOD_MAX(var_dv[VS], var_dv[VT]));
    lod = log2i(lod);
    if (lod > texture0->max_lod) lod = texture0->max_lod;

    t0_texels = texture0->lod[lod].texels;
    t0_alpha = texture0->lod[lod].alpha;
    t0_s_mask = texture0->lod[lod].s_mask;
    t0_t_mask = texture0->lod[lod].t_mask;
    t0_t_shift = texture0->lod[lod].t_shift;

    var_v[VS] >>= lod;
    var_dv[VS] >>= lod;

    var_v[VT] >>= lod;
    var_dv[VT] >>= lod;

#ifdef R_PERSPECTIVE
    var_ddv[VS] >>= lod;
    var_ddv[VT] >>= lod;
#endif

#endif

    n--;

    if (dx > 0) n -= dx;
    if (context->xmax + 1 < fixed_toint_round(x_right))
        n -= (fixed_toint_round(x_right) - (context->xmax + 1));

                while (n--) {



#ifdef R_DEPTH_TEST
                    if (R_DEPTH_FUNC(var_v[VZ])) {
#endif

#ifdef R_TEXTURE0
                    	t0_offset = TEXTURE2D_OFFSET;
#endif

#ifdef R_ALPHA_TEST
#ifdef R_TEXTURE0
                    if (t0_alpha[t0_offset] != alpha_value) {
#else
                    if (toAlpha(fixed_toint(var_v[VA])) != alpha_value) {
#endif
#endif

#ifdef R_TEXTURE0
                        fragment_color = t0_texels[t0_offset];
#ifdef R_TEXTURE0_MODULATE
#ifdef R_GREY_COLOR
                        fragment_color = (fragment_color | (fragment_color << 16)) & 0x07E0F81F;
                        fragment_color *= ((var_v[VG] >> 19) + 1);
                        fragment_color = (fragment_color >> 5) & 0x07E0F81F;
                        fragment_color |= (fragment_color >> 16);
                        fragment_color &= 0xFFFF;
#else
                        fragment_color = MODULATE_COLOR(fragment_color);
#endif
#endif
#ifdef R_BLEND
                        fragment_alpha = t0_alpha[t0_offset];
#ifdef R_TEXTURE0_MODULATE
                        fragment_alpha = MODULATE_ALPHA(fragment_alpha);
#endif
#endif

#else
                        fragment_color = ((var_v[VR] >> 8) & 0xF800) | ((var_v[VG] >> 13) & 0x07E0) | ((var_v[VB] >> 19) & 0x001F);
#ifdef R_BLEND
                        fragment_alpha = toAlpha(fixed_toint(var_v[VA]));
#endif

#endif


#ifdef R_BLEND
                        fragment_color = (fragment_color | (fragment_color << 16)) & 0x07E0F81F;
                        dest_color = (GLushort) *framebuffer;
                        dest_color = (dest_color | (dest_color << 16)) & 0x07E0F81F;
                        fragment_color = ((((fragment_color -  dest_color) * fragment_alpha) >> 5) + dest_color) & 0x07E0F81F;
                        *framebuffer = fragment_color | (fragment_color >> 16);
#else
                        *framebuffer = fragment_color;
#endif


                        *zbuffer = var_v[VZ];

#ifdef R_ALPHA_TEST
                    }
#endif

#ifdef R_DEPTH_TEST
                    }
#endif


                    zbuffer++;

                    framebuffer++;

#ifdef R_PERSPECTIVE
                    for (v = MAX_LINEAR_VAR;v < MAX_VAR;v++) {
                        var_v[v] += var_dv[v];
                        var_dv[v] += var_ddv[v];
                    }
                    for (v = 0;v < MAX_LINEAR_VAR;v++) var_v[v] += var_dv[v];
#else
                    for (v = 0;v < MAX_VAR;v++) var_v[v] += var_dv[v];
#endif

                } // while

            } // n != 0

        } // i >= ymin

        x_left += dxdy_left;
        x_right += dxdy_right;

#ifdef R_PERSPECTIVE
        p_1w_left += p_1w_dleft;
        p_1w_right += p_1w_dright;
#endif
        for (v = 0;v < MAX_VAR;v++) {
            var_left[v] += var_dleft[v];
            var_right[v] += var_dright[v];
        }
    }
}
#undef R_FLAT_COLOR
#undef R_GREY_COLOR
#undef R_ALPHA_TEST
#undef R_DEPTH_TEST
#undef R_AFFINE
#undef R_PERSPECTIVE
#undef R_BLEND
#undef R_TEXTURE0
#undef R_TEXTURE0_REPLACE
#undef R_TEXTURE0_MODULATE
#undef R_LOD

#undef MAX_VAR
#undef MAX_LINEAR_VAR
#undef VZ
#undef VR
#undef VG
#undef VB
#undef VA
#undef VS
#undef VT
