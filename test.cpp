#include "atpg.h"

#ifndef CONFLICT
#define CONFLICT 2
#endif
void ATPG::test(void) {
  string vec;
  int current_detect_num = 0;
  int total_detect_num = 0;
  int total_no_of_backtracks = 0;  // accumulative number of backtracks
  int current_backtracks = 0;
  int no_of_aborted_faults = 0;
  int no_of_redundant_faults = 0;
  int no_of_calls = 0;
  int j;
  vector<int> v2, v1;

//  fptr fault_under_test = flist_undetect.front();

  /* Fsim only mode */
  if(fsim_only)
    {
      fault_simulate_vectors(total_detect_num);
      in_vector_no+=vectors.size();
      display_undetect();
      fprintf(stdout,"\n");
      return;
    }// if fsim only
  
  if(tdfsim_only)
    {
      transition_delay_fault_simulation(total_detect_num);
      in_vector_no+=vectors.size();
      display_undetect();
  
      printf("\n# Result:\n");
      printf("-----------------------\n");
      printf("# total transition delay faults: %d\n", num_of_tdf_fault);
      printf("# total detected faults: %d\n", total_detect_num);
      printf("# fault coverage: %lf %\n", (float)total_detect_num / (float)num_of_tdf_fault * 100);
      return;
    }// if fsim only

#if 1
  /* LOS ATPG of TDF */

  /* ATPG mode */
  /* Figure 5 in the PODEM paper */

  // TODO 3.5.1 DTC (Ryan)
  // do { 
  fptr fault_under_test = flist_undetect.front();

  while(fault_under_test != nullptr  /* TODO 6: for all fault, repeat 1~5 */ ) {
    
    // FOR TODO 6 // for(int i = 0; i < this->ndet; i++){
    
    v1.clear();
    v2.clear();
    /* TODO 1 (DONE): take one fault from init_flist(use the generate_fault_list from PA3), generate the v2 pattern by PODEM (backtrack to PI and PPI) 
       STF <= STUCK1 , STR <= STUCK0 
       podem(fault_under_test,current_backtracks)
    */
    
    int podem_state = podem(fault_under_test,current_backtracks);
    for(j = 0; j < cktin.size(); j ++){
      v2.push_back(cktin[j]->value);
      v1.push_back(cktin[j]->value);
    }
    /* TODO 2: shift back one clock and mark the PI value*/

    v1.insert(v1.begin(), U);
  
    /* TODO 3: backtrack and generate v1 pattern (PI and PPI) */
    char ft[4]="STX"; 
    if(fault_under_test->fault_type == 0) ft[2]='R';
    else ft[2]='F';

    for(j = 0; j < cktin.size(); j ++){
      cktin[j]->value = v1[j];
    }
    
    int back_imply_result;
    back_imply_result = backward_imply(sort_wlist[fault_under_test->to_swlist], fault_under_test->fault_type); 
    if(back_imply_result == TRUE){
      v1[0] = cktin[0]->value;
    }
    else if(back_imply_result == FALSE){
      // In case of backward_imply returns FLASE or CONFLICT, we'll see later
      // Backtrack(direct imply) didn't reach the PI, need to assign PI by some decision.
      // sim() --> create a new find_pi_assignment() 
      // like in the while loop at line 53, podem.cpp 
        printf("backward_imply is FALSE ... We don't know yet how te deal with it :\'(\n");
        printf("failed fault type %s at %s \n", ft , sort_wlist[fault_under_test->to_swlist]->name.c_str() );
    }
    else if(back_imply_result == CONFLICT){
      // The test pattern contradict itself. There is no test for this fault.
        printf("backward imply conflict\n");
        printf("failed fault type %s at %s \n", ft , sort_wlist[fault_under_test->to_swlist]->name.c_str() );
    }
    /* V1 and V2 printing for test */
     printf("fault type %s at %s \n", ft , sort_wlist[fault_under_test->to_swlist]->name.c_str() );
    printf("\n V1 = ");
    for(int v: v1){
      printf("%d", v);
    }
    printf("\n V2 = ");
    for(int v: v2){
      printf("%d", v);
    }
    printf("\n");
    
    /* TODO 3.5 Dynamic Test Compression */ 
    // TODO 3.5.2 (Ryan)
    // }while(some PI is U);
    // TODO 3.5.3 (Ryan)
    // set all the input wire to U

    switch(  podem_state/* check if the test pattern is generated */  ) {
    case TRUE:
      /* form a vector */
      vec.clear();
      for (wptr w: cktin) {
	vec.push_back(itoc(w->value));
      }
      /*by defect, we want only one pattern per fault */
      /*run a fault simulation, drop ALL detected faults */
      if (total_attempt_num == 1) {
  
	/* TODO 4: check if other faults are detected by tdfsim() */

	fault_sim_a_vector(vec, current_detect_num);
	total_detect_num += current_detect_num;

      }
      /* If we want mutiple petterns per fault, 
       * NO fault simulation.  drop ONLY the fault under test */ 
      else {

	/* TODO 5 (DONE) : increase nb_of_detect, if nb_of_detect == ndet, remove fault from init_flist */
	  

	fault_under_test->detect = TRUE;
	fault_under_test->detected_time ++;
	/* drop fault_under_test if it has bit detected the enough amount of times*/
	if (fault_under_test->detected_time >= this->ndet){
	  flist_undetect.remove(fault_under_test);
	}


      }
      in_vector_no++;
      break;
    case FALSE:
      fault_under_test->detect = REDUNDANT;
      no_of_redundant_faults++;
      break;
  
    case MAYBE:
      no_of_aborted_faults++;
      break;
    }
    fault_under_test->test_tried = true;
    fault_under_test = nullptr;
    for (fptr fptr_ele: flist_undetect) {
      if (!fptr_ele->test_tried) {
        fault_under_test = fptr_ele;
        break;
      }
    }
    total_no_of_backtracks += current_backtracks; // accumulate number of backtracks
    no_of_calls++;
  }
  /* TODO 7: Static Test Compression*/
  // 7.1 Gathers all the test patterns
  // 7.2 Simulate for each pattern, (similar to PA3) (reversed order in which the patterns are generated)
  //   7.2.1 simulate v1 (activate the fault)
  //   7.2.2 simulate v2 (excite the fault and propagate to PO)
  // 7.3 Mark detected fault( and how many times it is detected)
  // 7.4 Drop the fault when the detected time reaches the goal.

#else
  /* ATPG mode */
  /* Figure 5 in the PODEM paper */
  while(fault_under_test != nullptr) {
    switch(podem(fault_under_test,current_backtracks)) {
    case TRUE:
      /* form a vector */
      vec.clear();
      for (wptr w: cktin) {
	vec.push_back(itoc(w->value));
      }
      /*by defect, we want only one pattern per fault */
      /*run a fault simulation, drop ALL detected faults */
      if (total_attempt_num == 1) {
	fault_sim_a_vector(vec, current_detect_num);
	total_detect_num += current_detect_num;
      }
      /* If we want mutiple petterns per fault, 
       * NO fault simulation.  drop ONLY the fault under test */ 
      else {
	fault_under_test->detect = TRUE;
	/* drop fault_under_test */
	flist_undetect.remove(fault_under_test);
      }
      in_vector_no++;
      break;
    case FALSE:
      fault_under_test->detect = REDUNDANT;
      no_of_redundant_faults++;
      break;
  
    case MAYBE:
      no_of_aborted_faults++;
      break;
    }
    fault_under_test->test_tried = true;
    fault_under_test = nullptr;
    for (fptr fptr_ele: flist_undetect) {
      if (!fptr_ele->test_tried) {
        fault_under_test = fptr_ele;
        break;
      }
    }
    total_no_of_backtracks += current_backtracks; // accumulate number of backtracks
    no_of_calls++;
  }
#endif

  display_undetect();
  fprintf(stdout,"\n");
  fprintf(stdout,"#number of aborted faults = %d\n",no_of_aborted_faults);
  fprintf(stdout,"\n");
  fprintf(stdout,"#number of redundant faults = %d\n",no_of_redundant_faults);
  fprintf(stdout,"\n");
  fprintf(stdout,"#number of calling podem1 = %d\n",no_of_calls);
  fprintf(stdout,"\n");
  fprintf(stdout,"#total number of backtracks = %d\n",total_no_of_backtracks);
}/* end of test */
