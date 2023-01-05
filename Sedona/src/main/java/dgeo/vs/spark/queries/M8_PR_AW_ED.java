package dgeo.vs.spark.queries;

import org.apache.sedona.core.spatialOperator.JoinQuery;
import org.apache.sedona.core.spatialRDD.SpatialRDD;
import org.apache.sedona.sql.utils.SedonaSQLRegistrator;
import org.apache.spark.api.java.JavaSparkContext;
import org.apache.spark.api.java.function.Function;
import org.apache.spark.storage.StorageLevel;
import org.apache.spark.sql.SparkSession;
import org.locationtech.jts.geom.Geometry;
import scala.Tuple2;

public class M8_PR_AW_ED {
    public static void main(String[] args) throws Exception {

        var spark = SparkSession.builder()
                .appName("PR x AW x ED sql")
                .config("spark.serializer", "org.apache.spark.serializer.KryoSerializer")
                .config("spark.kryo.registrator", "org.apache.sedona.core.serde.SedonaKryoRegistrator")
                .getOrCreate();

        SedonaSQLRegistrator.registerAll(spark);

        var jsc = new JavaSparkContext(spark.sparkContext());
        jsc.setLogLevel("WARN");
        jsc.hadoopConfiguration().set("fs.s3a.access.key", "xxxxxxxx");
        jsc.hadoopConfiguration().set("fs.s3a.secret.key", "xxxxxxxx");
        jsc.hadoopConfiguration().set("fs.s3a.endpoint", "s3.amazonaws.com");

        var loading_start = System.currentTimeMillis();

        var PR_WKT = "s3a://geods/proads.csv";
        var AW_WKT = "s3a://geods/areawater.csv";
        var ED_WKT = "s3a://geods/edges3.csv";

        var PR = Load.loadWKTsql(spark, PR_WKT, null);
        var AW = Load.loadWKTsql(spark, AW_WKT, PR.getPartitioner());
        var ED = Load.loadWKTsql(spark, ED_WKT, PR.getPartitioner());

        var loading_end = System.currentTimeMillis();
        var join_start = loading_end;

        System.out.println("Load ended.\n");

        var result_pr_aw = JoinQuery.SpatialJoinQueryFlat(
                PR, AW, true, true
        );
        System.out.printf("Intermediate has %d objects\n", result_pr_aw.count());

        var inter_start = System.currentTimeMillis();
        var result_m = result_pr_aw.map(new Function<Tuple2<Geometry, Geometry>, Geometry>() {
            @Override
            public Geometry call(Tuple2<Geometry, Geometry> g) throws Exception {
                return g._1();
            }
        });
        var result_m_rdd = new SpatialRDD<Geometry>();
        result_m_rdd.setRawSpatialRDD(result_m);
        result_m_rdd.analyze(StorageLevel.MEMORY_ONLY());
        result_m_rdd.spatialPartitioning(PR.getPartitioner());
        var inter_end = System.currentTimeMillis();

        var result = JoinQuery.SpatialJoinQueryFlat(
                result_m_rdd, ED, true, true);
        var cresult = result.count();

        var join_end = System.currentTimeMillis();
        System.out.printf("Join resulted %d tuples\n", cresult);
        System.out.printf("Load %d ms, Join %d ms, Intermed %d ms.\n",
                loading_end-loading_start,
                join_end-join_start,
                inter_end-inter_start);

    }
}
