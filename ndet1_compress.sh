./atpg -tdfatpg -ndet 1 -compression ../sample_circuits/c432.ckt > ../../pa3_2018/final_tdf_patterns_1c/c432.pat
./atpg -tdfatpg -ndet 1 -compression ../sample_circuits/c499.ckt > ../../pa3_2018/final_tdf_patterns_1c/c499.pat
./atpg -tdfatpg -ndet 1 -compression ../sample_circuits/c880.ckt > ../../pa3_2018/final_tdf_patterns_1c/c880.pat
./atpg -tdfatpg -ndet 1 -compression ../sample_circuits/c1355.ckt > ../../pa3_2018/final_tdf_patterns_1c/c1355.pat
./atpg -tdfatpg -ndet 1 -compression ../sample_circuits/c2670.ckt > ../../pa3_2018/final_tdf_patterns_1c/c2670.pat
./atpg -tdfatpg -ndet 1 -compression ../sample_circuits/c3540.ckt > ../../pa3_2018/final_tdf_patterns_1c/c3540.pat
./atpg -tdfatpg -ndet 1 -compression ../sample_circuits/c6288.ckt > ../../pa3_2018/final_tdf_patterns_1c/c6288.pat
./atpg -tdfatpg -ndet 1 -compression ../sample_circuits/c7552.ckt > ../../pa3_2018/final_tdf_patterns_1c/c7552.pat
../../pa3_2018/bin/golden_tdfsim -ndet 1 -tdfsim ../../pa3_2018/final_tdf_patterns_1c/c432.pat ../sample_circuits/c432.ckt   > ../gold_tdfsim_rpt/c432.rpt
../../pa3_2018/bin/golden_tdfsim -ndet 1 -tdfsim ../../pa3_2018/final_tdf_patterns_1c/c499.pat ../sample_circuits/c499.ckt   > ../gold_tdfsim_rpt/c499.rpt
../../pa3_2018/bin/golden_tdfsim -ndet 1 -tdfsim ../../pa3_2018/final_tdf_patterns_1c/c880.pat ../sample_circuits/c880.ckt   > ../gold_tdfsim_rpt/c880.rpt
../../pa3_2018/bin/golden_tdfsim -ndet 1 -tdfsim ../../pa3_2018/final_tdf_patterns_1c/c1355.pat ../sample_circuits/c1355.ckt > ../gold_tdfsim_rpt/c1355.rpt
../../pa3_2018/bin/golden_tdfsim -ndet 1 -tdfsim ../../pa3_2018/final_tdf_patterns_1c/c2670.pat ../sample_circuits/c2670.ckt > ../gold_tdfsim_rpt/c2670.rpt
../../pa3_2018/bin/golden_tdfsim -ndet 1 -tdfsim ../../pa3_2018/final_tdf_patterns_1c/c3540.pat ../sample_circuits/c3540.ckt > ../gold_tdfsim_rpt/c3540.rpt
../../pa3_2018/bin/golden_tdfsim -ndet 1 -tdfsim ../../pa3_2018/final_tdf_patterns_1c/c6288.pat ../sample_circuits/c6288.ckt > ../gold_tdfsim_rpt/c6288.rpt
../../pa3_2018/bin/golden_tdfsim -ndet 1 -tdfsim ../../pa3_2018/final_tdf_patterns_1c/c7552.pat ../sample_circuits/c7552.ckt > ../gold_tdfsim_rpt/c7552.rpt



for f in ../../pa3_2018/final_tdf_patterns_1c/* 
do
    echo "***"
    echo ${f##*/}
    grep "TEST LENGTH" $f
    grep "total gate fault cover" $f
    grep "cputime" $f | grep "test pattern"
done

echo "--golden tdf sim--"
for rp in ../gold_tdfsim_rpt/*
do
    echo "***"
    echo ${rp##*/}
    grep "fault cover" $rp 
done
