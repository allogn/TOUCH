/*
 * Huge function for total comparison run
 * 
 * Most of values are defined
 */

void fullComparison()
{
    /*
     * Save statistics in pretty .csv file with all useful data available
     * 
     * Note: for different algorithm if some data is absent - put zero
     */
    
        cout<<"Logging to file ";
        ofstream fout(tablefilename.c_str(),ios_base::app);
        
        // headers
        fout
        << "Epsilon, #A, #B, infile A, infile B, LocalJoin Alg, Fanout, Partitions, " // common parameters
        << "T Compared #, T Compared %, T Duplicates, T Results, T Selectivity, T filtered," // TOUCH
        << "T t loading, T t init, T t build, T t probe, T t compating, T t partition, T t join, T t total, T t deDuplicating, T t analyzing, T t sorting,"
        
        << "dT Compared #, dT Compared %, dT Duplicates, dT Results, dT Selectivity, dT filtered," // dTOUCH
        << "dT t loading, dT t init, dT t build, dT t probe, dT t compating, dT t partition, dT t join, T t total, dT t deDuplicating, dT t analyzing, dT t sorting,"
                
        << "cT Compared #, cT Compared %, cT Duplicates, cT Results, cT Selectivity, cT filtered," // cTOUCH
        << "cT t loading, cT t init, cT t build, cT t probe, cT t compating, cT t partition, cT t join, cT t total, cT t deDuplicating, cT t analyzing, cT t sorting,"
                
        << "reT Compared #, reT Compared %, reT Duplicates, reT Results, reT Selectivity, reT filtered," // reTOUCH
        << "reT t loading, reT t init, reT t build, reT t probe, reT t compating, reT t partition, reT t join, reT t total, reT t deDuplicating, reT t analyzing, reT t sorting\n";
                
        for 
                
                
        fout << algoname << " using " << basealgo << " gridSize " << gridSize
        << " epsilon " << epsilon
        << " memFP(MB) " << (footprint+0.0)/(1024.0*1024.0)
        << " #A " << size_dsA
        << " #B " << size_dsB
        << " sizeObj(B) " << sizeof(SpatialObjectList)
        << " EmptyCells(%) " << percentageEmpty
        << " MaxObj " << maxMappedObjects
        << " AveObj " << avg
        << " StdObj " << std

        << " Compared # " << ItemsCompared
        << " % " << 100 * (double)(ItemsCompared) / (double)(size_dsA * size_dsB)
        << " Duplicates " << resultPairs.duplicates
        << " Results " << resultPairs.results
        << " Selectivity " << 100.0*(double)resultPairs.results/(double)(size_dsA*size_dsB)
        << " filtered A:" << filtered[0] << " B " << filtered[1]
        << " repA " << repA
        << " repB " << repB

        << " Time "
        << " loading " << dataLoad
        << " init " << initialize
        << " build " << building
        << " probe " << probing
        << " comparing " << comparing
        << " partition " << partition
        << " join " << Ljoin
        << " total " << total
        << " deDuplicating " << resultPairs.deDuplicateTime
        << " analyzing " << analyzing
        << " sorting " << sorting

        << " partitions " << partitions
        << " Fanout " << base
        << " datasets " << file_dsA << " " << file_dsB;


    fout<< endl;
    fout.close();
}
