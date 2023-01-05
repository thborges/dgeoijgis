#ifndef OGREXT_H
#define OGREXT_H

#ifdef __cplusplus
extern "C" {
#endif

#include <geos_c.h>
#include <ogr_api.h>
#include <emmintrin.h>
#include <assert.h>

struct Envelope { 
			double MinX;
			double MinY;
			double MaxX;
			double MaxY;
};

typedef struct Envelope Envelope;

extern Envelope emptymbr;

/*Returns the intersection envelope of r an s*/
const Envelope EnvelopeIntersection(const Envelope r, const Envelope s);
const Envelope EnvelopeIntersection2(const Envelope r, const Envelope s);

/*Convert a Envelope to a OGRGeometry*/
OGRGeometryH EnvelopeToGeometry(const Envelope r);

void GetEnvelopeCoordinates(Envelope p, double *MinX, double *MinY, double *MaxX, double *MaxY);
Envelope GetEnvelope(double MinX, double MinY, double MaxX, double MaxY);

void GEOSGetEnvelope(const GEOSGeometry *geo, Envelope *env);

GEOSGeometry *OGR_G_ExportToGEOS( const OGRGeometryH ogrg);

Envelope OGRGetEnvelope(OGRGeometryH ogrg);
int OGRGetNumPoints(OGRGeometryH ogrg);

#define ENVELOPE_MERGE(mbr, e) do { \
mbr.MinX = MIN(mbr.MinX, e.MinX); \
mbr.MinY = MIN(mbr.MinY, e.MinY); \
mbr.MaxX = MAX(mbr.MaxX, e.MaxX); \
mbr.MaxY = MAX(mbr.MaxY, e.MaxY); } while(0)

#define ENVELOPE_BUFFER(mbr, sizew, sizeh) \
mbr.MinX -= sizew; \
mbr.MinY -= sizeh; \
mbr.MaxX += sizew; \
mbr.MaxY += sizeh

#define ENVELOPE_INIT(mbr) mbr = emptymbr

#define ENVELOPE_AREA(mbr) ((mbr.MaxX - mbr.MinX) * (mbr.MaxY - mbr.MinY))

#define ENVELOPE_PERIMETER(mbr) (2*(mbr.MaxX - mbr.MinX) + 2*(mbr.MaxY - mbr.MinY))

void print_geojson_mbr_local(const Envelope e, const char *id);

static inline __attribute__((always_inline))
char ENVELOPE_INTERSECTS(const Envelope mbr1, const Envelope mbr2) {
	return (mbr1.MinX <= mbr2.MaxX && mbr1.MinY <= mbr2.MaxY && mbr2.MinX <= mbr1.MaxX && mbr2.MinY <= mbr1.MaxY);
}

#define ENVELOPE_CONTAINS(o, i) (i.MinX >= o.MinX && i.MaxX <= o.MaxX && i.MinY >= o.MinY && i.MaxY <= o.MaxY)

#define ENVELOPE_CONTAINS_PNT(ev, x, y) (ev.MinX <= x && ev.MaxX >= x && ev.MinY <= y && ev.MaxY >= y)

#ifdef __cplusplus
}
#endif

#endif

