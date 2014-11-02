#include "QueryGenerator.cpp"

int main(int argc, const char* argv[])
{
	/*
	//WORLD  Low: (-1517.34,-1001.21,-1622.23)  High: (3395.09,1868.9,3423.49)
	//CENTER Low: (-57.7622,0,-69.8558)  High: (1792.5,1519.93,1803.42)
	 *
	 * 100K Densest Area
	 * Low (250,1200,200)
	 * High(1500,1400,1600)

     Box world = Box(Vertex(-1698.38,-1065.03,-1724.75),
    			    Vertex( 2248.56, 1894.28, 2276.71));
	 //double neuron_low[3] = {98.2538, 1005.14, 92.9046};
	 //double neuron_high[3] = {452.301, 1385.56, 456.743};
	 */

	std::string output = argv[1];
	float volume = atof(argv[2]);
	int queries = atoi(argv[3]);


    dias::QueryGenerator qg;

    dias::Box queryextent
            = dias::Box(dias::Vertex(250,1200,200),
                        dias::Vertex(1500,1400,1600));

   /* // queries for density analysis
    float width = (queryextent.high[2] - queryextent.low[2])/100;

    for (int i=0;i<100;i++)
    {
    	dias::Query query;
    	query.box.low[0]=-1517.34;
    	query.box.low[1]=-1001.21;
    	query.box.low[2]=queryextent.low[2]+(i*width);

    	query.box.high[0]=1792.5;
    	query.box.high[1]=1868.9;
    	query.box.high[2]=queryextent.low[2]+(i*width)+width;
    	qg.queries.push_back(query);
    }
*/
    qg.Generate(queryextent,queries,volume);
	qg.Save(argv[1]);

}
