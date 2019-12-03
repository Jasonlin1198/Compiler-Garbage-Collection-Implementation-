open Runner
open Printf
open OUnit2

let default_heap_size = "100000"

let tgc_main name heap_size program expected =
  name>::test_run program name expected [string_of_int heap_size]
let tgc_i_main name heap_size program expected args =
  name>::test_run program name expected ([string_of_int heap_size]@args)
let tgc_prog name heap_size program expected =
  name>::test_run program name expected [string_of_int heap_size]
let tgcerr_main name heap_size program expected =
  name>::test_err program name expected [string_of_int heap_size]

let t_i_prog name program expected args =
  name>::test_run program name expected (default_heap_size::args)
let t_prog name program expected =
  name>::test_run program name expected [default_heap_size]
let t_i_main name program expected args =
  name>::test_run program name expected (default_heap_size::args)
let t_main name program expected =
  name>::test_run program name expected [default_heap_size]
let terr_i_prog name program expected args =
  name>::test_err program name expected (default_heap_size::args)
let terr_prog name program expected =
  name>::test_err program name expected [default_heap_size]
let terr_i_main name program expected args =
  name>::test_err program name expected (default_heap_size::args)
let terr_main name program expected =
  name>::test_err program name expected [default_heap_size]
let t_f test_type = (fun (name,program,expected) ->
    test_type name program expected)
let f_to_s fname = Runner.string_of_file ("input/" ^ fname ^ ".boa");;

let lst_main body = (f_to_s "list") ^ body;;
let bst_main body = (f_to_s "bst") ^ body;;

(* All of thse are correct tests -- if they fail, something is broken *)
let data_tup = "(data tup (Num Num))\n(data tt (Num tup))\n";;
let oom =
  [
    tgcerr_main "oomgc1" 7 (data_tup ^ "(tt 1 (tup 3 4))") "Out of memory";
    tgc_main "oomgc2" 10 (data_tup ^ "(tt 1 (tup 3 4))") "(tt 1 (tup 3 4))";
    tgc_main "oomgc3" 10 (data_tup ^ "(tt 1 (tup 3 4))") "(tt 1 (tup 3 4))";
    tgc_main "oomgc4" 5 (data_tup ^ "(tup 3 4)") "(tup 3 4)";
    tgcerr_main "oomgc5" 3 (data_tup ^ "(tup 3 4)") "Allocation";
  ]

let gc_tests = [
  tgc_prog "overwrite_var" 10 (data_tup ^
  "(let ((t (tup 1 2)))
       (set t (tup 3 4))
       (set t (tup 5 6))
       (set t (tup 7 8))
       t)")
  "(tup 7 8)"
]

(* This is just to show you how you can write tests that use bst and lst *)
let bst_test = [
  t_prog "insert" (bst_main "(insert (insert (null Node) 5 4) 6 7)") "(Node (KV 5 4) null (Node (KV 6 7) null null))"
]


(* Add tests here as you write them! *)


let suite =
  "suite">:::
  oom @ gc_tests @ bst_test @ [


(*  tgc_main "first" 10 "(data Point (Num Num)) (data PLink (Point PLink)) (let ((p1 (Point 4 5)) (p2 (PLink p1 (null PLink)))) p2)" "0";

  tgc_main "checkpoint" 10000 "(data List (Num Bool)) (data Linked (List List Linked)) (let ((l1 (List 1 true)) (l2 (List 2 false)) (lnk1 (Linked l1 l2 (null Linked)))) lnk1)" "0";   
*)


]

let () =
  run_test_tt_main suite
;;

