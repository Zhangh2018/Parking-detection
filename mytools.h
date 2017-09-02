#include "stdafx.h"
#include "opencv2/imgproc.hpp"
#include "opencv2/highgui.hpp"
#include<algorithm>
#include <vector>
//#include "opencv2/calib3d/calib3d.hpp"
using namespace cv;
using namespace std;
using std::vector;
extern vector<Point2i> CenterPoints0;//所有匹配对象的重心

//用于聚类的工具1
bool _EqPredicate1(const cv::Point& a, const cv::Point&  b);
//用于聚类的工具2
bool _EqPredicate2(const cv::Point& a, const cv::Point&  b);
//Rosenfeld图像细化
void ThinnerRosenfeld(Mat& BinaryImg, Mat& ThinImg);
//区域生长算法去除小面积区域
void RemoveSmallRegion(Mat& Src, Mat& Dst, int AreaLimit, int CheckMode, int NeihborMode);
//将俯视图中的点映射回俯视变换前的图像上
cv::Point2i MapToSrc(short Bird_x, short Bird_y, short SrcWidth, short Srcheight);
//用于将图像变为俯视图
void TansformToBirdView(Mat& Src, Mat& Dst, double fx, double fy, double cx, double cy, double pitch, double h);
//基于畸变系数表的畸变校正
void CcalibrationByLable(Mat& Src, Mat& Dst, double fx, double fy, double cx, double cy);

const double realHeight[2001] = { 0, 0.001123637, 0.002247276, 0.003370915, 0.004494557, 0.005618201, 0.006741849, 0.007865501, 0.008989158, 0.01011282,
0.011236488, 0.012360163, 0.013483846, 0.014607537, 0.015731237, 0.016854947, 0.017978666, 0.019102397, 0.02022614, 0.021349894,
0.022473662, 0.023597444, 0.02472124, 0.025845051, 0.026968877, 0.028092721, 0.029216581, 0.030340459, 0.031464356, 0.032588272,
0.033712207, 0.034836163, 0.035960141, 0.03708414, 0.038208162, 0.039332207, 0.040456276, 0.04158037, 0.042704489, 0.043828634,
0.044952806, 0.046077005, 0.047201232, 0.048325487, 0.049449772, 0.050574087, 0.051698433, 0.05282281, 0.053947219, 0.055071661,
0.056196136, 0.057320646, 0.05844519, 0.05956977, 0.060694385, 0.061819038, 0.062943727, 0.064068455, 0.065193222, 0.066318028,
0.067442874, 0.068567761, 0.06969269, 0.07081766, 0.071942673, 0.07306773, 0.074192831, 0.075317976, 0.076443167, 0.077568404,
0.078693688, 0.079819019, 0.080944398, 0.082069826, 0.083195303, 0.084320831, 0.085446409, 0.086572038, 0.087697719, 0.088823453,
0.08994924, 0.091075081, 0.092200977, 0.093326927, 0.094452934, 0.095578997, 0.096705118, 0.097831296, 0.098957533, 0.100083829,
0.101210184, 0.1023366, 0.103463077, 0.104589616, 0.105716217, 0.106842881, 0.107969608, 0.1090964, 0.110223257, 0.111350179,
0.112477167, 0.113604222, 0.114731344, 0.115858535, 0.116985794, 0.118113122, 0.11924052, 0.120367989, 0.121495529, 0.12262314,
0.123750824, 0.124878582, 0.126006412, 0.127134317, 0.128262297, 0.129390352, 0.130518484, 0.131646692, 0.132774977, 0.133903341,
0.135031783, 0.136160304, 0.137288905, 0.138417586, 0.139546348, 0.140675192, 0.141804118, 0.142933127, 0.144062219, 0.145191396,
0.146320656, 0.147450002, 0.148579434, 0.149708953, 0.150838558, 0.151968251, 0.153098031, 0.154227901, 0.15535786, 0.156487909,
0.157618049, 0.158748279, 0.159878602, 0.161009016, 0.162139524, 0.163270125, 0.164400819, 0.165531609, 0.166662493, 0.167793474,
0.168924551, 0.170055724, 0.171186995, 0.172318364, 0.173449832, 0.174581398, 0.175713065, 0.176844832, 0.177976699, 0.179108668,
0.180240739, 0.181372913, 0.18250519, 0.18363757, 0.184770054, 0.185902643, 0.187035338, 0.188168138, 0.189301045, 0.190434059,
0.19156718, 0.192700409, 0.193833747, 0.194967194, 0.19610075, 0.197234417, 0.198368195, 0.199502083, 0.200636084, 0.201770197,
0.202904423, 0.204038762, 0.205173215, 0.206307782, 0.207442465, 0.208577263, 0.209712177, 0.210847208, 0.211982356, 0.213117621,
0.214253004, 0.215388506, 0.216524128, 0.217659868, 0.21879573, 0.219931711, 0.221067814, 0.222204039, 0.223340385, 0.224476855,
0.225613448, 0.226750164, 0.227887005, 0.22902397, 0.230161061, 0.231298277, 0.232435619, 0.233573088, 0.234710685, 0.235848409,
0.236986261, 0.238124242, 0.239262352, 0.240400591, 0.241538961, 0.242677461, 0.243816093, 0.244954855, 0.24609375, 0.247232778,
0.248371938, 0.249511232, 0.25065066, 0.251790222, 0.252929919, 0.254069751, 0.255209719, 0.256349824, 0.257490065, 0.258630443,
0.259770959, 0.260911613, 0.262052406, 0.263193338, 0.264334409, 0.26547562, 0.266616971, 0.267758463, 0.268900097, 0.270041872,
0.27118379, 0.27232585, 0.273468053, 0.2746104, 0.27575289, 0.276895525, 0.278038305, 0.279181231, 0.280324301, 0.281467518,
0.282610882, 0.283754393, 0.284898051, 0.286041857, 0.287185811, 0.288329914, 0.289474166, 0.290618568, 0.291763119, 0.292907821,
0.294052674, 0.295197678, 0.296342834, 0.297488142, 0.298633603, 0.299779216, 0.300924983, 0.302070904, 0.303216978, 0.304363208,
0.305509592, 0.306656132, 0.307802827, 0.308949679, 0.310096687, 0.311243852, 0.312391175, 0.313538655, 0.314686294, 0.315834092,
0.316982048, 0.318130164, 0.319278439, 0.320426875, 0.321575471, 0.322724228, 0.323873147, 0.325022227, 0.32617147, 0.327320875,
0.328470442, 0.329620174, 0.330770068, 0.331920127, 0.333070351, 0.334220739, 0.335371292, 0.336522011, 0.337672896, 0.338823947,
0.339975165, 0.34112655, 0.342278102, 0.343429822, 0.34458171, 0.345733767, 0.346885993, 0.348038388, 0.349190953, 0.350343688,
0.351496593, 0.352649669, 0.353802916, 0.354956334, 0.356109924, 0.357263687, 0.358417622, 0.35957173, 0.360726011, 0.361880466,
0.363035095, 0.364189898, 0.365344875, 0.366500028, 0.367655356, 0.36881086, 0.36996654, 0.371122397, 0.37227843, 0.37343464,
0.374591028, 0.375747594, 0.376904337, 0.37806126, 0.379218361, 0.380375641, 0.381533101, 0.382690741, 0.383848561, 0.385006562,
0.386164743, 0.387323106, 0.38848165, 0.389640376, 0.390799284, 0.391958375, 0.393117649, 0.394277106, 0.395436747, 0.396596572,
0.397756581, 0.398916774, 0.400077153, 0.401237716, 0.402398465, 0.4035594, 0.404720522, 0.40588183, 0.407043324, 0.408205006,
0.409366876, 0.410528933, 0.411691179, 0.412853613, 0.414016235, 0.415179047, 0.416342049, 0.41750524, 0.418668622, 0.419832193,
0.420995956, 0.42215991, 0.423324055, 0.424488391, 0.42565292, 0.426817641, 0.427982555, 0.429147662, 0.430312962, 0.431478456,
0.432644144, 0.433810026, 0.434976103, 0.436142374, 0.437308841, 0.438475503, 0.439642361, 0.440809415, 0.441976666, 0.443144113,
0.444311758, 0.4454796, 0.446647639, 0.447815877, 0.448984313, 0.450152948, 0.451321781, 0.452490814, 0.453660047, 0.454829479,
0.455999112, 0.457168945, 0.458338979, 0.459509214, 0.46067965, 0.461850289, 0.463021129, 0.464192172, 0.465363417, 0.466534865,
0.467706517, 0.468878372, 0.470050432, 0.471222695, 0.472395163, 0.473567836, 0.474740714, 0.475913797, 0.477087087, 0.478260582,
0.479434284, 0.480608192, 0.481782307, 0.48295663, 0.48413116, 0.485305898, 0.486480845, 0.487656, 0.488831364, 0.490006937,
0.491182719, 0.492358711, 0.493534914, 0.494711326, 0.49588795, 0.497064784, 0.49824183, 0.499419087, 0.500596556, 0.501774238,
0.502952132, 0.504130239, 0.505308559, 0.506487092, 0.50766584, 0.508844801, 0.510023977, 0.511203368, 0.512382973, 0.513562794,
0.514742831, 0.515923083, 0.517103552, 0.518284237, 0.519465139, 0.520646259, 0.521827596, 0.523009151, 0.524190924, 0.525372915,
0.526555125, 0.527737554, 0.528920203, 0.530103071, 0.53128616, 0.532469468, 0.533652997, 0.534836748, 0.536020719, 0.537204912,
0.538389327, 0.539573965, 0.540758824, 0.541943907, 0.543129213, 0.544314742, 0.545500495, 0.546686473, 0.547872674, 0.549059101,
0.550245753, 0.55143263, 0.552619732, 0.553807061, 0.554994616, 0.556182398, 0.557370407, 0.558558643, 0.559747107, 0.560935799,
0.562124719, 0.563313868, 0.564503246, 0.565692853, 0.566882689, 0.568072756, 0.569263053, 0.57045358, 0.571644339, 0.572835328,
0.57402655, 0.575218003, 0.576409688, 0.577601606, 0.578793757, 0.579986141, 0.581178759, 0.582371611, 0.583564697, 0.584758017,
0.585951572, 0.587145363, 0.588339389, 0.589533651, 0.59072815, 0.591922885, 0.593117857, 0.594313066, 0.595508513, 0.596704197,
0.59790012, 0.599096282, 0.600292683, 0.601489323, 0.602686203, 0.603883322, 0.605080682, 0.606278283, 0.607476125, 0.608674208,
0.609872533, 0.6110711, 0.61226991, 0.613468962, 0.614668258, 0.615867797, 0.61706758, 0.618267607, 0.619467879, 0.620668395,
0.621869157, 0.623070165, 0.624271418, 0.625472918, 0.626674665, 0.627876658, 0.629078899, 0.630281388, 0.631484125, 0.632687111,
0.633890345, 0.635093829, 0.636297562, 0.637501545, 0.638705779, 0.639910263, 0.641114999, 0.642319986, 0.643525224, 0.644730715,
0.645936459, 0.647142455, 0.648348705, 0.649555208, 0.650761965, 0.651968977, 0.653176244, 0.654383766, 0.655591543, 0.656799576,
0.658007866, 0.659216412, 0.660425216, 0.661634277, 0.662843595, 0.664053172, 0.665263008, 0.666473103, 0.667683457, 0.668894071,
0.670104945, 0.671316079, 0.672527475, 0.673739132, 0.67495105, 0.676163231, 0.677375674, 0.678588381, 0.67980135, 0.681014583,
0.682228081, 0.683441843, 0.684655869, 0.685870162, 0.687084719, 0.688299543, 0.689514634, 0.690729991, 0.691945616, 0.693161509,
0.694377669, 0.695594098, 0.696810797, 0.698027764, 0.699245001, 0.700462509, 0.701680287, 0.702898336, 0.704116657, 0.705335249,
0.706554114, 0.707773251, 0.708992662, 0.710212346, 0.711432304, 0.712652537, 0.713873044, 0.715093826, 0.716314884, 0.717536218,
0.718757829, 0.719979716, 0.721201881, 0.722424324, 0.723647045, 0.724870044, 0.726093323, 0.727316881, 0.728540719, 0.729764838,
0.730989237, 0.732213917, 0.73343888, 0.734664124, 0.735889651, 0.737115461, 0.738341554, 0.739567931, 0.740794593, 0.742021539,
0.743248771, 0.744476288, 0.745704091, 0.746932181, 0.748160558, 0.749389223, 0.750618175, 0.751847416, 0.753076945, 0.754306764,
0.755536872, 0.756767271, 0.757997961, 0.759228941, 0.760460213, 0.761691777, 0.762923634, 0.764155784, 0.765388227, 0.766620963,
0.767853995, 0.769087321, 0.770320942, 0.771554859, 0.772789072, 0.774023582, 0.775258389, 0.776493494, 0.777728897, 0.778964598,
0.780200599, 0.781436899, 0.782673499, 0.783910399, 0.7851476, 0.786385103, 0.787622908, 0.788861015, 0.790099425, 0.791338139,
0.792577156, 0.793816478, 0.795056104, 0.796296036, 0.797536273, 0.798776817, 0.800017668, 0.801258826, 0.802500291, 0.803742065,
0.804984148, 0.806226539, 0.807469241, 0.808712253, 0.809955575, 0.811199209, 0.812443155, 0.813687412, 0.814931983, 0.816176866,
0.817422064, 0.818667575, 0.819913401, 0.821159543, 0.822406, 0.823652774, 0.824899864, 0.826147272, 0.827394997, 0.828643041,
0.829891403, 0.831140085, 0.832389087, 0.833638409, 0.834888051, 0.836138016, 0.837388302, 0.83863891, 0.839889842, 0.841141096,
0.842392675, 0.843644578, 0.844896807, 0.846149361, 0.84740224, 0.848655447, 0.84990898, 0.851162842, 0.852417031, 0.853671549,
0.854926396, 0.856181573, 0.85743708, 0.858692918, 0.859949088, 0.861205589, 0.862462422, 0.863719589, 0.864977089, 0.866234923,
0.867493092, 0.868751595, 0.870010434, 0.87126961, 0.872529122, 0.873788971, 0.875049158, 0.876309683, 0.877570548, 0.878831751,
0.880093295, 0.881355179, 0.882617404, 0.883879971, 0.885142879, 0.886406131, 0.887669725, 0.888933664, 0.890197946, 0.891462574,
0.892727547, 0.893992866, 0.895258532, 0.896524544, 0.897790904, 0.899057613, 0.90032467, 0.901592076, 0.902859832, 0.904127939,
0.905396397, 0.906665206, 0.907934367, 0.909203881, 0.910473748, 0.911743969, 0.913014544, 0.914285474, 0.915556759, 0.916828401,
0.918100399, 0.919372754, 0.920645467, 0.921918538, 0.923191968, 0.924465757, 0.925739907, 0.927014417, 0.928289288, 0.92956452,
0.930840115, 0.932116073, 0.933392394, 0.934669079, 0.935946129, 0.937223543, 0.938501324, 0.93977947, 0.941057984, 0.942336864,
0.943616113, 0.94489573, 0.946175717, 0.947456073, 0.948736799, 0.950017896, 0.951299365, 0.952581206, 0.953863419, 0.955146005,
0.956428965, 0.9577123, 0.958996009, 0.960280094, 0.961564554, 0.962849392, 0.964134607, 0.965420199, 0.96670617, 0.96799252,
0.969279249, 0.970566359, 0.971853849, 0.97314172, 0.974429974, 0.975718609, 0.977007628, 0.978297031, 0.979586818, 0.980876989,
0.982167546, 0.983458489, 0.984749819, 0.986041535, 0.98733364, 0.988626132, 0.989919014, 0.991212285, 0.992505946, 0.993799998,
0.995094442, 0.996389277, 0.997684504, 0.998980125, 1.000276139, 1.001572547, 1.002869351, 1.004166549, 1.005464144, 1.006762135,
1.008060523, 1.009359309, 1.010658494, 1.011958077, 1.01325806, 1.014558443, 1.015859227, 1.017160412, 1.018461999, 1.019763989,
1.021066381, 1.022369178, 1.023672378, 1.024975983, 1.026279994, 1.027584411, 1.028889235, 1.030194465, 1.031500104, 1.032806151,
1.034112607, 1.035419473, 1.036726748, 1.038034435, 1.039342533, 1.040651043, 1.041959965, 1.043269301, 1.04457905, 1.045889214,
1.047199792, 1.048510787, 1.049822197, 1.051134024, 1.052446268, 1.05375893, 1.055072011, 1.05638551, 1.05769943, 1.059013769,
1.06032853, 1.061643711, 1.062959315, 1.064275342, 1.065591791, 1.066908665, 1.068225963, 1.069543685, 1.070861834, 1.072180408,
1.073499409, 1.074818838, 1.076138694, 1.077458979, 1.078779693, 1.080100837, 1.081422411, 1.082744416, 1.084066852, 1.08538972,
1.086713021, 1.088036755, 1.089360923, 1.090685525, 1.092010562, 1.093336035, 1.094661943, 1.095988289, 1.097315071, 1.098642292,
1.099969951, 1.101298049, 1.102626586, 1.103955564, 1.105284983, 1.106614843, 1.107945144, 1.109275888, 1.110607076, 1.111938707,
1.113270782, 1.114603302, 1.115936267, 1.117269679, 1.118603536, 1.119937841, 1.121272594, 1.122607795, 1.123943444, 1.125279543,
1.126616092, 1.127953092, 1.129290542, 1.130628445, 1.131966799, 1.133305607, 1.134644867, 1.135984582, 1.137324751, 1.138665375,
1.140006455, 1.141347991, 1.142689984, 1.144032434, 1.145375342, 1.146718708, 1.148062534, 1.149406819, 1.150751564, 1.152096769,
1.153442436, 1.154788565, 1.156135156, 1.15748221, 1.158829728, 1.160177709, 1.161526155, 1.162875066, 1.164224443, 1.165574286,
1.166924595, 1.168275372, 1.169626617, 1.17097833, 1.172330512, 1.173683164, 1.175036285, 1.176389878, 1.177743941, 1.179098476,
1.180453483, 1.181808962, 1.183164915, 1.184521342, 1.185878243, 1.187235618, 1.18859347, 1.189951797, 1.1913106, 1.192669881,
1.194029638, 1.195389874, 1.196750589, 1.198111782, 1.199473455, 1.200835608, 1.202198242, 1.203561357, 1.204924954, 1.206289032,
1.207653594, 1.209018638, 1.210384166, 1.211750179, 1.213116676, 1.214483658, 1.215851126, 1.217219081, 1.218587522, 1.21995645,
1.221325866, 1.222695771, 1.224066164, 1.225437046, 1.226808418, 1.22818028, 1.229552633, 1.230925477, 1.232298813, 1.233672641,
1.235046962, 1.236421776, 1.237797084, 1.239172885, 1.240549182, 1.241925973, 1.24330326, 1.244681043, 1.246059323, 1.247438099,
1.248817373, 1.250197145, 1.251577415, 1.252958184, 1.254339453, 1.255721221, 1.257103489, 1.258486259, 1.259869529, 1.261253301,
1.262637575, 1.264022352, 1.265407631, 1.266793414, 1.268179701, 1.269566492, 1.270953788, 1.272341589, 1.273729896, 1.275118709,
1.276508028, 1.277897855, 1.279288188, 1.28067903, 1.28207038, 1.283462238, 1.284854606, 1.286247483, 1.28764087, 1.289034767,
1.290429176, 1.291824095, 1.293219526, 1.294615469, 1.296011924, 1.297408893, 1.298806374, 1.300204369, 1.301602878, 1.303001902,
1.304401441, 1.305801494, 1.307202064, 1.308603149, 1.310004751, 1.311406869, 1.312809505, 1.314212658, 1.315616329, 1.317020519,
1.318425227, 1.319830454, 1.321236201, 1.322642467, 1.324049254, 1.325456561, 1.326864389, 1.328272739, 1.329681609, 1.331091002,
1.332500918, 1.333911355, 1.335322316, 1.3367338, 1.338145808, 1.33955834, 1.340971397, 1.342384978, 1.343799084, 1.345213715,
1.346628872, 1.348044556, 1.349460765, 1.350877501, 1.352294764, 1.353712555, 1.355130873, 1.356549719, 1.357969093, 1.359388996,
1.360809428, 1.362230389, 1.363651879, 1.365073899, 1.366496449, 1.367919529, 1.36934314, 1.370767282, 1.372191955, 1.373617159,
1.375042896, 1.376469164, 1.377895964, 1.379323297, 1.380751163, 1.382179562, 1.383608494, 1.38503796, 1.386467959, 1.387898493,
1.389329561, 1.390761163, 1.392193301, 1.393625973, 1.395059181, 1.396492924, 1.397927203, 1.399362018, 1.400797369, 1.402233257,
1.403669681, 1.405106642, 1.40654414, 1.407982176, 1.409420749, 1.41085986, 1.412299508, 1.413739695, 1.41518042, 1.416621683,
1.418063486, 1.419505827, 1.420948707, 1.422392126, 1.423836085, 1.425280583, 1.426725621, 1.428171199, 1.429617317, 1.431063975,
1.432511173, 1.433958912, 1.435407192, 1.436856013, 1.438305374, 1.439755277, 1.441205721, 1.442656706, 1.444108233, 1.445560302,
1.447012912, 1.448466064, 1.449919758, 1.451373994, 1.452828773, 1.454284094, 1.455739957, 1.457196363, 1.458653311, 1.460110802,
1.461568836, 1.463027413, 1.464486533, 1.465946196, 1.467406402, 1.468867151, 1.470328444, 1.47179028, 1.473252659, 1.474715582,
1.476179048, 1.477643058, 1.479107611, 1.480572708, 1.482038349, 1.483504534, 1.484971262, 1.486438534, 1.48790635, 1.489374709,
1.490843613, 1.49231306, 1.493783052, 1.495253587, 1.496724666, 1.498196289, 1.499668456, 1.501141166, 1.502614421, 1.504088219,
1.505562561, 1.507037447, 1.508512877, 1.50998885, 1.511465367, 1.512942428, 1.514420032, 1.51589818, 1.517376872, 1.518856107,
1.520335885, 1.521816206, 1.523297071, 1.524778479, 1.52626043, 1.527742925, 1.529225962, 1.530709542, 1.532193664, 1.53367833,
1.535163537, 1.536649288, 1.538135581, 1.539622416, 1.541109793, 1.542597712, 1.544086173, 1.545575175, 1.54706472, 1.548554805,
1.550045432, 1.551536601, 1.55302831, 1.55452056, 1.556013351, 1.557506682, 1.559000554, 1.560494966, 1.561989918, 1.56348541,
1.564981442, 1.566478013, 1.567975123, 1.569472772, 1.570970961, 1.572469688, 1.573968953, 1.575468757, 1.576969098, 1.578469978,
1.579971395, 1.581473349, 1.582975841, 1.584478869, 1.585982434, 1.587486535, 1.588991173, 1.590496346, 1.592002055, 1.593508299,
1.595015078, 1.596522392, 1.59803024, 1.599538623, 1.601047539, 1.602556989, 1.604066972, 1.605577489, 1.607088538, 1.608600119,
1.610112232, 1.611624877, 1.613138053, 1.614651761, 1.616165999, 1.617680767, 1.619196066, 1.620711894, 1.622228251, 1.623745137,
1.625262552, 1.626780495, 1.628298966, 1.629817964, 1.631337489, 1.632857541, 1.634378119, 1.635899223, 1.637420852, 1.638943006,
1.640465685, 1.641988888, 1.643512615, 1.645036865, 1.646561638, 1.648086934, 1.649612751, 1.65113909, 1.65266595, 1.65419333,
1.655721231, 1.657249651, 1.658778591, 1.660308049, 1.661838025, 1.663368519, 1.664899531, 1.666431059, 1.667963103, 1.669495663,
1.671028738, 1.672562328, 1.674096432, 1.675631049, 1.67716618, 1.678701823, 1.680237977, 1.681774644, 1.683311821, 1.684849508,
1.686387705, 1.687926411, 1.689465626, 1.691005349, 1.692545579, 1.694086315, 1.695627558, 1.697169306, 1.69871156, 1.700254317,
1.701797578, 1.703341342, 1.704885609, 1.706430377, 1.707975647, 1.709521417, 1.711067686, 1.712614455, 1.714161722, 1.715709487,
1.71725775, 1.718806508, 1.720355762, 1.721905512, 1.723455755, 1.725006492, 1.726557722, 1.728109445, 1.729661658, 1.731214363,
1.732767557, 1.734321241, 1.735875413, 1.737430072, 1.738985219, 1.740540852, 1.74209697, 1.743653573, 1.74521066, 1.74676823,
1.748326282, 1.749884816, 1.75144383, 1.753003324, 1.754563297, 1.756123749, 1.757684677, 1.759246083, 1.760807964, 1.762370319,
1.763933149, 1.765496452, 1.767060227, 1.768624474, 1.770189191, 1.771754377, 1.773320033, 1.774886156, 1.776452746, 1.778019802,
1.779587323, 1.781155308, 1.782723757, 1.784292668, 1.78586204, 1.787431873, 1.789002165, 1.790572916, 1.792144124, 1.793715788,
1.795287909, 1.796860484, 1.798433512, 1.800006993, 1.801580926, 1.803155309, 1.804730142, 1.806305424, 1.807881153, 1.809457328,
1.811033949, 1.812611014, 1.814188523, 1.815766474, 1.817344866, 1.818923698, 1.820502969, 1.822082678, 1.823662825, 1.825243406,
1.826824423, 1.828405873, 1.829987755, 1.831570069, 1.833152813, 1.834735986, 1.836319586, 1.837903614, 1.839488067, 1.841072944,
1.842658245, 1.844243967, 1.84583011, 1.847416673, 1.849003655, 1.850591053, 1.852178868, 1.853767097, 1.85535574, 1.856944795,
1.858534261, 1.860124137, 1.861714421, 1.863305113, 1.864896211, 1.864896211, 1.864896211, 1.872857744, 1.872857744, 1.872857744,
1.87445125, 1.876045153, 1.877639452, 1.879234145, 1.880829231, 1.882424708, 1.884020575, 1.885616831, 1.887213475, 1.888810504,
1.890407918, 1.892005715, 1.893603894, 1.895202454, 1.896801392, 1.898400708, 1.9000004, 1.901600466, 1.903200906, 1.904801718,
1.906402899, 1.90800445, 1.909606368, 1.911208651, 1.912811299, 1.91441431, 1.916017681, 1.917621413, 1.919225503, 1.92082995,
1.922434751, 1.924039907, 1.925645414, 1.927251272, 1.928857478, 1.930464032, 1.932070932, 1.933678175, 1.935285761, 1.936893688,
1.938501954, 1.940110558, 1.941719497, 1.943328771, 1.944938378, 1.946548315, 1.948158582, 1.949769176, 1.951380096, 1.95299134,
1.954602907, 1.956214794, 1.957827001, 1.959439524, 1.961052364, 1.962665517, 1.964278981, 1.965892757, 1.96750684, 1.96912123,
1.970735925, 1.972350923, 1.973966223, 1.975581821, 1.977197717, 1.978813909, 1.980430395, 1.982047172, 1.98366424, 1.985281596,
1.986899238, 1.988517165, 1.990135374, 1.991753864, 1.993372632, 1.994991677, 1.996610997, 1.99823059, 1.999850453, 2.001470585,
2.003090985, 2.004711649, 2.006332576, 2.007953763, 2.00957521, 2.011196913, 2.012818872, 2.014441082, 2.016063544, 2.017686254,
2.01930921, 2.020932411, 2.022555854, 2.024179537, 2.025803458, 2.027427616, 2.029052006, 2.030676629, 2.03230148, 2.033926559,
2.035551863, 2.03717739, 2.038803138, 2.040429104, 2.042055286, 2.043681682, 2.04530829, 2.046935107, 2.048562131, 2.05018936,
2.051816792, 2.053444424, 2.055072254, 2.056700279, 2.058328498, 2.059956908, 2.061585506, 2.06321429, 2.064843259, 2.066472408,
2.068101737, 2.069731242, 2.071360922, 2.072990773, 2.074620793, 2.076250981, 2.077881332, 2.079511845, 2.081142518, 2.082773348,
2.084404331, 2.086035467, 2.087666752, 2.089298183, 2.090929759, 2.092561476, 2.094193332, 2.095825324, 2.09745745, 2.099089707,
2.100722093, 2.102354604, 2.103987238, 2.105619993, 2.107252865, 2.108885853, 2.110518952, 2.112152161, 2.113785477, 2.115418898,
2.117052419, 2.118686039, 2.120319754, 2.121953562, 2.123587461, 2.125221447, 2.126855517, 2.128489668, 2.130123898, 2.131758204,
2.133392583, 2.135027031, 2.136661547, 2.138296127, 2.139930767, 2.141565466, 2.14320022, 2.144835026, 2.146469882, 2.148104783,
2.149739727, 2.151374712, 2.153009733, 2.154644788, 2.156279874, 2.157914987, 2.159550125, 2.161185285, 2.162820462, 2.164455655,
2.166090859, 2.167726072, 2.16936129, 2.170996511, 2.17263173, 2.174266946, 2.175902153, 2.17753735, 2.179172533, 2.180807698,
2.182442842, 2.184077962, 2.185713054, 2.187348116, 2.188983143, 2.190618132, 2.19225308, 2.193887984, 2.195522839, 2.197157643,
2.198792392, 2.200427082, 2.20206171, 2.203696272, 2.205330765, 2.206965186, 2.20859953, 2.210233793, 2.211867974, 2.213502067,
2.215136069, 2.216769977, 2.218403786, 2.220037493, 2.221671095, 2.223304587, 2.224937966, 2.226571228, 2.228204369, 2.229837386,
2.231470274, 2.23310303, 2.234735649, 2.236368129, 2.238000465, 2.239632653, 2.241264689, 2.242896569, 2.24452829, 2.246159847,
2.247791236, 2.249422454, 2.251053496, 2.252684358, 2.254315036, 2.255945526, 2.257575825, 2.259205926, 2.260835828, 2.262465525,
2.264095013, 2.265724288, 2.267353346, 2.268982183, 2.270610794, 2.272239175, 2.273867321, 2.275495229, 2.277122894, 2.278750312,
2.280377478, 2.282004388, 2.283631037, 2.285257422, 2.286883537, 2.288509378, 2.290134941, 2.291760221, 2.293385213, 2.295009913,
2.296634317, 2.29825842, 2.299882216, 2.301505703, 2.303128874, 2.304751725, 2.306374252, 2.30799645, 2.309618314, 2.311239839,
2.312861021, 2.314481854, 2.316102334, 2.317722456, 2.319342216, 2.320961607, 2.322580626, 2.324199268, 2.325817527, 2.327435399,
2.329052878, 2.330669959, 2.332286638, 2.33390291, 2.335518769, 2.33713421, 2.338749228, 2.340363818, 2.341977975, 2.343591694,
2.345204969, 2.346817796, 2.348430168, 2.350042081, 2.35165353, 2.353264508, 2.354875011, 2.356485034, 2.358094571, 2.359703616,
2.361312164, 2.36292021, 2.364527749, 2.366134774, 2.36774128, 2.369347262, 2.370952714, 2.372557631, 2.374162006, 2.375765835,
2.377369112, 2.37897183, 2.380573985, 2.382175571, 2.383776581, 2.38537701, 2.386976853, 2.388576103, 2.390174754, 2.391772802,
2.393370239, 2.394967061, 2.39656326, 2.398158832, 2.399753769, 2.401348067, 2.402941719, 2.404534719, 2.406127062, 2.40771874,
2.409309748, 2.410900079, 2.412489729, 2.414078689, 2.415666955, 2.41725452, 2.418841378, 2.420427522, 2.422012946, 2.423597644,
2.425181609, 2.426764836, 2.428347317, 2.429929047, 2.431510019, 2.433090226, 2.434669662, 2.434669662, 2.434669662, 2.434669662,
2.434669662, 2.44727661, 2.44727661, 2.44727661, 2.44727661, 2.448848812, 2.450420176, 2.451990697, 2.453560368, 2.455129181,
2.45669713, 2.458264208, 2.459830409, 2.461395725, 2.462960149, 2.464523675, 2.466086296, 2.467648004, 2.469208793, 2.470768656,
2.472327585, 2.473885574, 2.475442616, 2.476998703, 2.478553829, 2.480107986, 2.481661167, 2.483213365, 2.484764573, 2.486314783,
2.487863989, 2.489412183, 2.490959357, 2.492505505, 2.494050619, 2.495594692, 2.497137716, 2.498679685, 2.50022059, 2.501760424,
2.50329918, 2.50483685, 2.506373427, 2.507908903, 2.509443271, 2.510976524, 2.512508653, 2.514039651, 2.51556951, 2.517098223,
2.518625783, 2.520152181, 2.521677409, 2.523201461, 2.524724328, 2.526246003, 2.527766478, 2.529285745, 2.530803796, 2.532320623,
2.53383622, 2.535350577, 2.536863687, 2.538375541, 2.539886134, 2.541395455, 2.542903497, 2.544410253, 2.545915715, 2.547419873,
2.548922721, 2.55042425, 2.551924453, 2.55342332, 2.554920845, 2.556417019, 2.557911834, 2.559405282, 2.560897354, 2.562388043,
2.56387734, 2.565365238, 2.566851728, 2.568336801, 2.569820451, 2.571302667, 2.572783443, 2.57426277, 2.575740639, 2.577217043,
2.578691973, 2.58016542, 2.581637377, 2.583107836, 2.584576787, 2.586044222, 2.587510133, 2.588974513, 2.590437351, 2.591898641,
2.593358373, 2.594816539, 2.596273131, 2.59772814, 2.599181558, 2.600633376, 2.602083587, 2.603532181, 2.60497915, 2.606424485,
2.607868179, 2.609310222, 2.610750606, 2.612189323, 2.613626364, 2.615061721, 2.616495385, 2.617927347, 2.619357599, 2.620786133,
2.62221294, 2.623638011, 2.625061338, 2.626482912, 2.627902726, 2.629320769, 2.630737035, 2.632151513, 2.633564196, 2.634975075,
2.636384142, 2.637791388, 2.639196803, 2.640600381, 2.642002113, 2.643401989, 2.644800001, 2.646196141, 2.6475904, 2.64898277,
2.650373242, 2.651761808, 2.653148458, 2.654533185, 2.65591598, 2.657296835, 2.65867574, 2.660052689, 2.661427671, 2.662800678,
2.664171703, 2.665540736, 2.666907769, 2.668272794, 2.669635802, 2.670996785, 2.672355734, 2.673712641, 2.675067497, 2.676420295,
2.677771025, 2.67911968, 2.680466251, 2.681810729, 2.683153107, 2.684493375, 2.685831527, 2.687167553, 2.688501444, 2.689833194,
2.691162793, 2.692490234, 2.693815508, 2.695138607, 2.696459522, 2.697778246, 2.699094771, 2.700409088, 2.701721189, 2.703031066,
2.704338712, 2.705644117, 2.706947274, 2.708248176, 2.709546813, 2.710843178, 2.712137264, 2.713429061, 2.714718563, 2.716005762,
2.717290648, 2.718573216, 2.719853457, 2.721131362, 2.722406926, 2.723680139, 2.724950994, 2.726219483, 2.727485599, 2.728749334,
2.730010681 };