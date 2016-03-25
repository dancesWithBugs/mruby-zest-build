#include <mruby.h>
#include <mruby/array.h>
#include <mruby/class.h>
#include <mruby/data.h>
#include <mruby/string.h>
#include <GL/gl.h>
#include "../../../deps/pugl/pugl/event.h"
#include "../../../deps/pugl/pugl/common.h"
#include "../../../deps/pugl/pugl/pugl.h"

static mrb_value
mrb_gl_viewport(mrb_state *mrb, mrb_value self)
{
    mrb_float x, y, w, h;
    mrb_get_args(mrb, "ffff", &x, &y, &w, &h);
    glViewport(x, y, w, h);
    return self;
}
static mrb_value
mrb_gl_clear_color(mrb_state *mrb, mrb_value self)
{
    mrb_float r, b, g, a;
    mrb_get_args(mrb, "ffff", &r, &b, &g, &a);
    glClearColor(r, b, g, a);
    return self;
}
static mrb_value
mrb_gl_clear(mrb_state *mrb, mrb_value self)
{
    mrb_int clear_mode;
    mrb_get_args(mrb, "i", &clear_mode);
    //glClear(clear_mode);
    glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT|GL_STENCIL_BUFFER_BIT);
    return self;
}

/*******************************************************************************
 *                          PUGL Code Here                                     *
 *                                                                             *
 ******************************************************************************/
static void
onReshape(PuglView* view, int width, int height)
{
    void **v = (void**)puglGetHandle(view);
    printf("reshape to %dx%d\n", width, height);
    if(v) {
        mrb_value obj = mrb_obj_value(v[1]);
        mrb_funcall(v[0], obj, "resize", 2, mrb_fixnum_value(width), mrb_fixnum_value(height));
    }
#if 0
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glViewport(0, 0, width, height);
	gluPerspective(45.0f, width/(float)height, 1.0f, 10.0f);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
#endif
}

static void
onDisplay(PuglView* view)
{
    void **v = (void**)puglGetHandle(view);
    printf("on-display...\n");
    if(v) {
        mrb_value obj = mrb_obj_value(v[1]);
        mrb_funcall(v[0], obj, "draw", 0);
    }
}

static void
printModifiers(PuglView* view)
{
	int mods = puglGetModifiers(view);
	fprintf(stderr, "Modifiers:%s%s%s%s\n",
	        (mods & PUGL_MOD_SHIFT) ? " Shift"   : "",
	        (mods & PUGL_MOD_CTRL)  ? " Ctrl"    : "",
	        (mods & PUGL_MOD_ALT)   ? " Alt"     : "",
	        (mods & PUGL_MOD_SUPER) ? " Super" : "");
}

static void
onEvent(PuglView* view, const PuglEvent* event)
{
	if (event->type == PUGL_KEY_PRESS) {
		const uint32_t ucode = event->key.character;
		fprintf(stderr, "Key %u (char %u) down (%s)%s\n",
		        event->key.keycode, ucode, event->key.utf8,
		        event->key.filter ? " (filtered)" : "");
	}
}

static void
onSpecial(PuglView* view, bool press, PuglKey key)
{
	fprintf(stderr, "Special key %d %s ", key, press ? "down" : "up");
	printModifiers(view);
}

static void
onMotion(PuglView* view, int x, int y)
{
	fprintf(stderr, "Mouse Move %d %d\n", x, y);
	//xAngle = x % 360;
	//yAngle = y % 360;
	puglPostRedisplay(view);
}

static void
onMouse(PuglView* view, int button, bool press, int x, int y)
{
    void **v = (void**)puglGetHandle(view);
	fprintf(stderr, "Mouse %d %s at %d,%d ",
	        button, press ? "down" : "up", x, y);
	printModifiers(view);
    if(v) {
        mrb_value obj = mrb_obj_value(v[1]);
        mrb_funcall(v[0], obj, "mouse", 4, 
                mrb_fixnum_value(button),
                mrb_fixnum_value(press),
                mrb_fixnum_value(x),
                mrb_fixnum_value(y));
    }
}

static void
onScroll(PuglView* view, int x, int y, float dx, float dy)
{
	fprintf(stderr, "Scroll %d %d %f %f ", x, y, dx, dy);
	printModifiers(view);
	//dist += dy / 4.0f;
	puglPostRedisplay(view);
}

static void
onClose(PuglView* view)
{
    void **v = (void**)puglGetHandle(view);
    if(v) {
        mrb_value obj = mrb_obj_value(v[1]);
        mrb_funcall(v[0], obj, "quit", 0);
    }
}

static void
mrb_pugl_free(mrb_state *mrb, void *ptr)
{
}

const struct mrb_data_type mrb_pugl_type = {"PUGL", mrb_pugl_free};

static mrb_value
mrb_pugl_tick(mrb_state *mrb, mrb_value self)
{
    PuglView *view = (PuglView*)mrb_data_get_ptr(mrb, self, &mrb_pugl_type);
    puglProcessEvents(view);
    return self;
}


static mrb_value
mrb_pugl_initialize(mrb_state *mrb, mrb_value self)
{
    PuglView *view = puglInit(0,0);
    //puglInitWindowClass(view, "PuglWindow");
	puglInitWindowSize(view, 512, 512);
    puglInitResizable(view, true);

	puglSetEventFunc(view, onEvent);
	puglSetMotionFunc(view, onMotion);
	puglSetMouseFunc(view, onMouse);
	puglSetScrollFunc(view, onScroll);
	puglSetSpecialFunc(view, onSpecial);
	puglSetDisplayFunc(view, onDisplay);
	puglSetReshapeFunc(view, onReshape);
	puglSetCloseFunc(view, onClose);

	puglCreateWindow(view, "Pugl Test");
	puglShowWindow(view);
    puglProcessEvents(view);

    mrb_data_init(self, view, &mrb_pugl_type);
    mrb_funcall(mrb, self, "w=", 1, mrb_fixnum_value(512));
    mrb_funcall(mrb, self, "h=", 1, mrb_fixnum_value(512));

    return self;
}

static mrb_value
mrb_pugl_size(mrb_state *mrb, mrb_value self)
{
    mrb_value ary = mrb_ary_new(mrb);
    mrb_sym xid = mrb_intern_str(mrb, mrb_str_new_cstr(mrb, "x"));
    mrb_sym yid = mrb_intern_str(mrb, mrb_str_new_cstr(mrb, "y"));
    mrb_ary_push(mrb, ary, mrb_attr_get(mrb, self, xid));
    mrb_ary_push(mrb, ary, mrb_attr_get(mrb, self, yid));
    return ary;
}

static mrb_value
mrb_pugl_size_set(mrb_state *mrb, mrb_value self)
{
    mrb_value ary;
    mrb_get_args(mrb, "o", &ary);
    mrb_value w = mrb_ary_ref(mrb, ary, 0);
    mrb_value h = mrb_ary_ref(mrb, ary, 1);
    mrb_funcall(mrb, self, "w=", 1, w);
    mrb_funcall(mrb, self, "h=", 1, h);
    return self;
}

static mrb_value
mrb_pugl_make_current(mrb_state *mrb, mrb_value self)
{
    PuglView *view = (PuglView*)mrb_data_get_ptr(mrb, self, &mrb_pugl_type);
    void puglEnterContext(PuglView* view);
    puglEnterContext(view);
    return self;
}

static mrb_value
mrb_pugl_should_close(mrb_state *mrb, mrb_value self)
{
    PuglView *view = (PuglView*)mrb_data_get_ptr(mrb, self, &mrb_pugl_type);
    //void puglEnterContext(PuglView* view);
    //puglEnterContext(view);
    return mrb_false_value();
}

static mrb_value
mrb_pugl_poll(mrb_state *mrb, mrb_value self)
{
    PuglView *view = (PuglView*)mrb_data_get_ptr(mrb, self, &mrb_pugl_type);
    puglProcessEvents(view);
    return mrb_false_value();
}


static mrb_value
mrb_pugl_impl(mrb_state *mrb, mrb_value self)
{
    PuglView *view = (PuglView*)mrb_data_get_ptr(mrb, self, &mrb_pugl_type);
    mrb_value zrunner;
    mrb_get_args(mrb, "o", &zrunner);
    void **v = mrb_malloc(mrb, 2*sizeof(void*));
    v[0] = mrb;
    v[1] = mrb_cptr(zrunner);
    puglSetHandle(view, v);
    return mrb_false_value();
}

static mrb_value
mrb_pugl_dummy(mrb_state *mrb, mrb_value self)
{
    PuglView *view = (PuglView*)mrb_data_get_ptr(mrb, self, &mrb_pugl_type);
    //void puglEnterContext(PuglView* view);
    //puglEnterContext(view);
    return self;
}



void
mrb_mruby_widget_lib_gem_init(mrb_state* mrb) {
    struct RClass *module = mrb_define_module(mrb, "GL");
    mrb_define_class_method(mrb, module, "gl_viewport",    mrb_gl_viewport,    MRB_ARGS_REQ(4));
    mrb_define_class_method(mrb, module, "gl_clear_color", mrb_gl_clear_color, MRB_ARGS_REQ(4));
    mrb_define_class_method(mrb, module, "gl_clear",       mrb_gl_clear,       MRB_ARGS_REQ(1));

    struct RClass *pugl = mrb_define_class_under(mrb, module, "PUGL", mrb->object_class);
    MRB_SET_INSTANCE_TT(pugl, MRB_TT_DATA);

    mrb_define_method(mrb, pugl, "initialize",   mrb_pugl_initialize,   MRB_ARGS_REQ(1));
    mrb_define_method(mrb, pugl, "make_current", mrb_pugl_make_current, MRB_ARGS_NONE());
    mrb_define_method(mrb, pugl, "should_close", mrb_pugl_should_close, MRB_ARGS_NONE());
    mrb_define_method(mrb, pugl, "size",         mrb_pugl_size,         MRB_ARGS_NONE());
    mrb_define_method(mrb, pugl, "size=",        mrb_pugl_size_set,     MRB_ARGS_REQ(1));
    mrb_define_method(mrb, pugl, "title=",       mrb_pugl_dummy,        MRB_ARGS_REQ(1));
    mrb_define_method(mrb, pugl, "poll",         mrb_pugl_poll,         MRB_ARGS_NONE());
    mrb_define_method(mrb, pugl, "impl=",        mrb_pugl_impl,         MRB_ARGS_REQ(1));

}

void
mrb_mruby_widget_lib_gem_final(mrb_state* mrb) {
    /* finalizer */
}
