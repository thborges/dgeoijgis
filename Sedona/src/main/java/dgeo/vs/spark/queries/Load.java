package dgeo.vs.spark.queries;

import org.apache.sedona.core.enums.GridType;
import org.apache.sedona.core.enums.IndexType;
import org.apache.sedona.core.formatMapper.WktReader;
import org.apache.sedona.core.formatMapper.shapefileParser.ShapefileReader;
import org.apache.sedona.core.spatialPartitioning.SpatialPartitioner;
import org.apache.sedona.core.spatialRDD.PointRDD;
import org.apache.sedona.core.spatialRDD.SpatialRDD;
import org.apache.sedona.sql.utils.Adapter;
import org.apache.spark.api.java.JavaSparkContext;
import org.apache.spark.sql.SparkSession;
import org.apache.spark.storage.StorageLevel;
import org.locationtech.jts.geom.Geometry;

public class Load {

    //static int loadPartitionNumber = 100;
    //static int loadPartitionNumber = 250;
    //static int loadPartitionNumber = 500;
    static int loadPartitionNumber = 1000;
    //static int loadPartitionNumber = 1500;
    //static int loadPartitionNumber = 2000;
    //static int loadPartitionNumber = 2500;
    //static int loadPartitionNumber = 3000;

    public static SpatialRDD<Geometry> loadShapeFile(JavaSparkContext jsc, String dataset_shp) throws Exception {
        return loadShapeFile(jsc, dataset_shp, null);
    }

    public static SpatialRDD<Geometry> loadShapeFile(JavaSparkContext jsc, String dataset_shp,
                                                     SpatialPartitioner partitioner) throws Exception {
        var dataset = ShapefileReader.readToGeometryRDD(jsc, dataset_shp);
        dataset.analyze(StorageLevel.MEMORY_ONLY());
        if (partitioner == null)
            dataset.spatialPartitioning(GridType.KDBTREE, loadPartitionNumber);
        else
            dataset.spatialPartitioning(partitioner);
        dataset.buildIndex(IndexType.RTREE, true);
        var ds_name = dataset_shp.substring(dataset_shp.lastIndexOf('/'));
        System.out.printf("\n%s has %d objects\n", ds_name, dataset.countWithoutDuplicates());
        return dataset;
    }

    public static SpatialRDD<Geometry> loadWKT(JavaSparkContext jsc, String dataset_wkt) throws Exception {
        return loadWKT(jsc, dataset_wkt, null);
    }

    public static SpatialRDD<Geometry> loadWKT(JavaSparkContext jsc, String dataset_wkt, SpatialPartitioner partitioner) throws Exception {
        var dataset = WktReader.readToGeometryRDD(jsc, dataset_wkt,
                0, false, true);
        var ds_name = dataset_wkt.substring(dataset_wkt.lastIndexOf('/'));
        System.out.printf("\n%s has %d objects\n", ds_name, dataset.countWithoutDuplicates());

        System.out.printf("Analyzing...\n");
        dataset.analyze(StorageLevel.MEMORY_ONLY());

        System.out.printf("Partitioning...\n");
        if (partitioner == null)
            dataset.spatialPartitioning(GridType.KDBTREE, loadPartitionNumber);
        else
            dataset.spatialPartitioning(partitioner);

        System.out.printf("Indexing...\n");
        dataset.buildIndex(IndexType.RTREE, true);

        System.out.print("Done.\n\n");
        return dataset;
    }

    public static SpatialRDD<Geometry> loadWKTsql(SparkSession spark, String dataset_wkt, SpatialPartitioner partitioner) throws Exception {

        var df = spark.read().format("csv")
                .option("header", "false")
                .option("delimiter", "\t").load(dataset_wkt);
        df.createOrReplaceTempView("temp");

        var sp = spark.sql(
                "select ST_GeomFromWKT(_c0) as geom from temp");
        sp.show(2);
        var ds_name = dataset_wkt.substring(dataset_wkt.lastIndexOf('/'));
        System.out.printf("\n%s has %d objects\n", ds_name, sp.count());

        System.out.printf("Converting to spatial RDD...\n");
        var dataset = Adapter.toSpatialRdd(sp, 0);

        System.out.printf("Analyzing...\n");
        dataset.analyze(StorageLevel.MEMORY_ONLY());

        System.out.printf("Partitioning...\n");
        if (partitioner == null)
            dataset.spatialPartitioning(GridType.KDBTREE, loadPartitionNumber);
        else
            dataset.spatialPartitioning(partitioner);

        System.out.printf("Indexing...\n");
        dataset.buildIndex(IndexType.RTREE, true);

        System.out.print("Done.\n\n");
        return dataset;
    }
}
