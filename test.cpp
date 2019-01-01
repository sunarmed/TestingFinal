#include "atpg.h"
#include <time.h>
#include <stdlib.h>

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
  int notest;
  int secondary_fault =0 ;
  forward_list<wptr> decision_tree; // design_tree (a LIFO stack)

  vector<int> v2, v1;
  vector<vector<int>> patterns;
  int cpt_vec = 0;

//  fptr fault_under_test = flist_undetect.front();
  srand(time(NULL));

 
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
  for( wptr w:sort_wlist){
        w->value = U;
  }
  /* ATPG mode */
  /* Figure 5 in the PODEM paper */

  // TODO 3.5.1 DTC (Ryan)
  // do {
  
  fptr fault_under_test = flist_undetect.front();

  while(fault_under_test != nullptr  /* TODO 6: for all fault, repeat 1~5 */ ) {

   
    /* Naming the fault */
    char ft[4]="STX"; 
    if(fault_under_test->fault_type == 0) ft[2]='R';
    else ft[2]='F';
    printf("********\nfault type %s at %s \n", ft , sort_wlist[fault_under_test->to_swlist]->name.c_str() );
  
 
    // FOR TODO 6 // for(int i = 0; i < this->ndet; i++){

    /* Initialize Circuits  */
    
    if(!secondary_fault){
      for( wptr w:sort_wlist){
          w->value = U;
      }
    }

    if(!secondary_fault){
        v1.clear();
        v2.clear();
    }
    /* TODO 1 (DONE): take one fault from init_flist(use the generate_fault_list from PA3), generate the v2 pattern by PODEM (backtrack to PI and PPI) 
       STF <= STUCK1 , STR <= STUCK0 
       podem(fault_under_test,current_backtracks)
    */
    int podem_state = podem(fault_under_test,current_backtracks,secondary_fault);

    if(podem_state == TRUE){
    /* find v2, put in vector v2 */
        for(j = 0; j < cktin.size(); j ++){
        
          if(!secondary_fault){
              v2.push_back(cktin[j]->value);
              v1.push_back(cktin[j]->value);
          }
          else{
              v2[j] = cktin[j]->value;
              v1[j+1] = cktin[j]->value;
          }
        }
    }
    else{
       no_test = true;  
       goto weird;
    }

    /* TODO 2: shift back one clock, put U at the first input of v1 */
    if(!secondary_fault){
        v1.insert(v1.begin(), U);
    }
    /* TODO 3: backtrack and generate v1 pattern (PI and PPI) */
   // for( wptr w:sort_wlist){
   //     w->value = U;
   // }
    /* mark PI with v1, simulate and see if the fault is activated  */
    for (j = 0; j < sort_wlist.size(); j++) {
        sort_wlist[j]->value = U;
    }
    for(j = 0; j < cktin.size(); j++){
      cktin[j]->value = v1[j];
      cktin[j]->flag |= CHANGED;
    }
    sim(); 
    
    //check fault activation?
    
   if( sort_wlist[fault_under_test->to_swlist]->value == (1 ^ fault_under_test->fault_type)  ){
        /* The target is impossible to be activated, no test */  
 	printf(" AAA test not exist \n");
        no_test = true;
    }
    else if(sort_wlist[fault_under_test->to_swlist]->value == U ){
        /* The target is still unknown at v1 input. Find new PI assignment*/
        /* The same as the PODEM "test possible" part */
        printf(" AAA test to be determined \n");
           #if 1
             no_of_backtracks = 0;
             find_test = false;
             no_test = false;
              wptr wpi;
             while ((no_of_backtracks < backtrack_limit) && !no_test &&
               !(find_test /*&& (attempt_num == total_attempt_num)*/)) {
               
               /* check if test possible.   Fig. 7.1 */
               /* find PI assignment based on v1. So, if there is an assigned value, check with the objective and skip its assignment.*/
               if (wpi = find_pi_assignment_for_v1( sort_wlist[fault_under_test->to_swlist] ,   fault_under_test->fault_type    )) {
    //             printf("--- find an assignment for v1\n");
                 wpi->flag |= CHANGED;
                 /* insert a new PI into decision_tree */
                 decision_tree.push_front(wpi);
               }
               else { // no test possible using this assignment, backtrack. 
    //             printf("--- not find an assignment for v1\n");
           
                 while (!decision_tree.empty() && (wpi == nullptr)) {
                   /* if both 01 already tried, backtrack. Fig.7.7 */
                   if (decision_tree.front()->flag & ALL_ASSIGNED) {
                     decision_tree.front()->flag &= ~ALL_ASSIGNED;  // clear the ALL_ASSIGNED flag
                     decision_tree.front()->value = U; // do not assign 0 or 1
                     decision_tree.front()->flag |= CHANGED; // this PI has been changed
                     /* remove this PI in decision tree.  see dashed nodes in Fig 6 */
                     decision_tree.pop_front();
                   }  
                   /* else, flip last decision, flag ALL_ASSIGNED. Fig. 7.8 */
                   else {
                     decision_tree.front()->value = decision_tree.front()->value ^ 1; // flip last decision
                     decision_tree.front()->flag |= CHANGED; // this PI has been changed
                     decision_tree.front()->flag |= ALL_ASSIGNED;
                     no_of_backtracks++;
                     wpi = decision_tree.front(); 
                   }
                 } // while decision tree && ! wpi
                 if (wpi == nullptr) no_test = true; //decision tree empty,  Fig 7.9
               } // no test possible
                  
           /* this again loop is to generate multiple patterns for a single fault 
            * this part is NOT in the original PODEM paper  */
           again:  if (wpi) {
                 /* sim after a new PI is assigned */
                 sim();
                 /* find test if the fault is activated (at v1) */
                 if (sort_wlist[fault_under_test->to_swlist]->value == fault_under_test->fault_type) {
                   find_test = true;
//                   printf("[line %d]find test after new pi assigment\n",__LINE__);
                   /* if multiple patterns per fault, print out every test cube */
//                     display_io(); 
                   for(j=0;j<cktin.size();j++){
                        v1[j] = cktin[j]->value;
                    }
                 }  // if check_test()
               } // again
             } // while (three conditions)
           
             /* clear everthing */
             for (wptr wptr_ele: decision_tree) {
               wptr_ele->flag &= ~ALL_ASSIGNED;
             }
             decision_tree.clear();
             
             current_backtracks = no_of_backtracks;
           //  unmark_propagate_tree(fault->node);
           #endif	
    }else if (sort_wlist[fault_under_test->to_swlist]->value == fault_under_test->fault_type ){
         /*v1 need no more assignment and the fault is activated */
         printf(" AAA test done \n");
   
    }else{
       printf(" unknown status? %s=%d \n" , sort_wlist[fault_under_test->to_swlist]->name.c_str(), sort_wlist[fault_under_test->to_swlist]->value );
    }
  

    if(no_test){
            printf("no test is true\n");
            podem_state = FALSE;
    }else{
            podem_state = TRUE;
    } 

    /* check if v2 has U in PO (for DTC) */
    for(j=0;j<cktin.size();j++){
        cktin[j]->value = v2[j];
        cktin[j]->flag |= CHANGED;
    } 
    secondary_fault = 0;
    sim();
    for(j=0;j<cktout.size();j++){
        printf(" %d ",cktout[j]->value);
        if(cktout[j]->value == U) secondary_fault = 1;
    }
    if(secondary_fault){
         printf("\nPO=U. Find  next secondary fault\n");
    } 
   
weird:
    /* V1 and V2 printing for test */
   printf("\n  V1 = ");
    for(int v: v1){
      printf("%d", v);
    }
    printf("\n  V2 = ");
    for(int v: v2){
      printf("%d", v);
    }
    printf("\n***\n");
//    patterns[cpt_vec++] = v1;
    
    /* TODO 3.5 Dynamic Test Compression */ 
    // TODO 3.5.2 
    // }while(some PO is U);
    // TODO 3.5.3 
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
      if (total_attempt_num == 999) {
  
	/* TODO 4: check if other faults are detected by tdfsim() */
	fault_under_test->detected_time ++;
	fault_sim_a_vector(vec, current_detect_num);
	total_detect_num += current_detect_num;

      }
      /* If we want mutiple petterns per fault, 
       * NO fault simulation.  drop ONLY the fault under test */ 
      else {

	/* TODO 5 (DONE) : increase nb_of_detect, if nb_of_detect == ndet, remove fault from init_flist */
	  
        printf("drop N-det fault \n");
	fault_under_test->detect = TRUE;
	fault_under_test->detected_time ++;
	/* drop fault_under_test if it has bit detected the enough amount of times*/
	if (fault_under_test->detected_time >= this->ndet){
	  flist_undetect.remove(fault_under_test);
	}


      }
      in_vector_no++;
      break;
   // case CONFLICT:
    case FALSE:
      if(!secondary_fault) {
           fault_under_test->detect = REDUNDANT;
           no_of_redundant_faults++;
      }
      break;
  
    case MAYBE:
      no_of_aborted_faults++;
      break;
    }
    printf("CCC test tried\n");
    fault_under_test->test_tried = true;
    fault_under_test = nullptr;
        for (fptr fptr_ele: flist_undetect) {
          if ( (rand()%5 == 0)  &&  (fptr_ele->detected_time < ndet)  && (fptr_ele->detect != REDUNDANT) ) {
            fault_under_test = fptr_ele;
            break;
          }
        }
    if(secondary_fault){
        printf("next secondary fault:");
    }
    else{
        printf("next primary fault:");
    }
//    fault_under_test = nullptr;
//    for (fptr fptr_ele: flist_undetect) {
//      if (!fptr_ele->test_tried) {
//        fault_under_test = fptr_ele;
//        break;
//      }
//    }
    total_no_of_backtracks += current_backtracks; // accumulate number of backtracks
    no_of_calls++;
  }
  /* TODO 7: Static Test Compression*/
  // 7.1 Gathers all the test patterns (DONE, in var patterns)
  // 7.2 Simulate for each pattern, (similar to PA3) (reversed order in which the patterns are generated)
  //for(vector<int> vec: patterns){
    // Sould we convert vectors into strings to use this functions ? 
    //tdfault_sim_a_vector(/* ???? */, current_detect_num);
    //vec.erase(vec.begin());
    //tdfault_sim_a_vector(/* ??? */, current_detect_num);
  //}
  //   7.2.1 simulate v1 (activate the fault)
  //   7.2.2 simulate v2 (excite the fault and propagate to PO)
  // 7.3 Mark detected fault( and how many times it is detected)
  // 7.4 Drop the fault when the detected time reaches the goal.


#else
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
