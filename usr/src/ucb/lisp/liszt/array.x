(File array.l)
(d-vectorindexcode lambda setq d-calltran e-write2 d-move quote e-label e-goto d-genlab let null cond if)
(d-vref lambda d-vectorindexcode e-goto not return caar null do assq d-handlecc e-cvt d-clearreg e-write4 concat eq e-label e-write2 cdr car prog1 pop |1-| decr d-fixnumexp incr setf push e-write3 d-move d-exp d-simple *throw throw |1+| terpri niceprint patom comp-msg progn comp-err <& fixp if quote list cons setq cond makecomment caddr cadr let)
(cc-vrefi-long lambda quote d-vref)
(cc-vrefi-word lambda quote d-vref)
(cc-vrefi-byte lambda quote d-vref)
(cc-vref lambda quote d-vref)
(d-arrayindexcomp lambda *throw throw |1+| terpri niceprint patom comp-msg progn comp-err apply list setq quote cons do length eq car cdr null cond If)
(d-dostore lambda |1-| decr d-move d-superrplacx car list rplaca ncons d-pushargs gensym cdr d-arrayindexcomp quote cons setq cond makecomment let)
(d-handlearrayref lambda d-supercxr list d-arrayindexcomp cddr cadr d-dostore cdr length |1+| eq If cons setq cond makecomment quote car get let)
