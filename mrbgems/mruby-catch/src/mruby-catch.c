#include <mruby.h>
#include <mruby/class.h>
#include <mruby/variable.h>
#include <mruby/error.h>
#include <mruby/proc.h>
#include <mruby/opcode.h>
#include <mruby/presym.h>


static const mrb_sym catch_syms_3[1] = {MRB_SYM(call),};
static const mrb_code catch_iseq_3[18] = {
  OP_ENTER,    0x00, 0x00, 0x00,
  OP_GETUPVAR, 0x02, 0x02, 0x01,
  OP_GETUPVAR, 0x03, 0x01, 0x01,
  OP_SEND,     0x02, 0x00, 0x01,
  OP_RETURN,   0x02,};
static const mrb_irep catch_irep_3 = {
  2,5,0,
  MRB_IREP_STATIC,catch_iseq_3,
  NULL,catch_syms_3,NULL,
  NULL,
  NULL,
  18,0,1,0,0
};
static const mrb_irep *catch_reps_2[1] = {
  &catch_irep_3,
};
static const mrb_code catch_iseq_2[13] = {
  OP_ENTER,    0x00, 0x00, 0x00,
  OP_LAMBDA,   0x02, 0x00,
  OP_SEND,     0x02, 0x00, 0x00,
  OP_RETURN,   0x02,};
static const mrb_irep catch_irep_2 = {
  2,4,0,
  MRB_IREP_STATIC,catch_iseq_2,
  NULL,catch_syms_3,catch_reps_2,
  NULL,
  NULL,
  13,0,1,1,0
};
static const mrb_irep *catch_reps_1[1] = {
  &catch_irep_2,
};
static const mrb_sym catch_syms_1[3] = {MRB_SYM(Object), MRB_SYM(new), MRB_SYM(call),};
static const mrb_code catch_iseq_1[29] = {
  OP_ENTER,    0x00, 0x20, 0x01,
  OP_JMP,      0x00, 0x03,
  OP_JMP,      0x00, 0x0a,
  OP_GETCONST, 0x03, 0x00,
  OP_SEND,     0x03, 0x01, 0x00,
  OP_MOVE,     0x01, 0x03,
  OP_LAMBDA,   0x03, 0x00,
  OP_SEND,     0x03, 0x02, 0x00,
  OP_RETURN,   0x03,};
static const mrb_irep catch_irep = {
  3,5,0,
  MRB_IREP_STATIC,catch_iseq_1,
  NULL,catch_syms_1,catch_reps_1,
  NULL,
  NULL,
  29,0,3,1,0
};

#define ID_PRESERVED_CATCH MRB_SYM(__preserved_catch_proc)

static const mrb_callinfo *
find_catcher(mrb_state *mrb, mrb_value tag)
{
  mrb_value pval = mrb_obj_iv_get(mrb, (struct RObject *)mrb->kernel_module, ID_PRESERVED_CATCH);
  mrb_assert(mrb_proc_p(pval));
  const struct RProc *proc = mrb_proc_ptr(pval);

  const mrb_callinfo *ci = mrb->c->ci;
  size_t n = ci - mrb->c->cibase;
  ci--;
  for (; n > 0; n--, ci--) {
    const mrb_value *arg1 = ci->stack + 1;
    if (ci->proc == proc && mrb_obj_eq(mrb, *arg1, tag)) {
      return ci;
    }
  }

  return NULL;
}

static mrb_value
mrb_f_throw(mrb_state *mrb, mrb_value self)
{
  mrb_value tag, obj;
  if (mrb_get_args(mrb, "o|o", &tag, &obj) == 1) {
    obj = mrb_nil_value();
  }

  const mrb_callinfo *ci = find_catcher(mrb, tag);
  if (ci) {
    struct RBreak *b = (struct RBreak *)mrb_obj_alloc(mrb, MRB_TT_BREAK, NULL);
    mrb_break_value_set(b, obj);
    mrb_break_proc_set(b, ci[2].proc); /* Back to the closure in `catch` method */
    mrb_exc_raise(mrb, mrb_obj_value(b));
  }
  else {
    mrb_value argv[2] = {tag, obj};
    mrb_exc_raise(mrb, mrb_obj_new(mrb, mrb_exc_get_id(mrb, MRB_ERROR_SYM(UncaughtThrowError)), 2, argv));
  }
  /* not reached */
  return mrb_nil_value();
}

void
mrb_mruby_catch_gem_init(mrb_state *mrb)
{
  struct RProc *p;
  mrb_method_t m;

  p = mrb_proc_new(mrb, &catch_irep);
  MRB_METHOD_FROM_PROC(m, p);
  mrb_define_method_raw(mrb, mrb->kernel_module, MRB_SYM(catch), m);
  mrb_obj_iv_set(mrb, (struct RObject *)mrb->kernel_module, ID_PRESERVED_CATCH, mrb_obj_value(p));

  mrb_define_method(mrb, mrb->kernel_module, "throw", mrb_f_throw, MRB_ARGS_ARG(1,1));
}

void
mrb_mruby_catch_gem_final(mrb_state *mrb)
{
}
