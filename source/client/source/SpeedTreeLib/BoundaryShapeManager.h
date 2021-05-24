
#include <vector>
#include <string>
struct SPoint
{
		float&      operator[](int i) { return m_afData[i]; }
		float       m_afData[3];
};
struct SBoundaryShape
{
		std::vector< std::vector<SPoint> >	m_vContours;
		float								m_afMin[3];
		float								m_afMax[3];
};
class CBoundaryShapeManager
{
public:
		CBoundaryShapeManager( );
		virtual ~CBoundaryShapeManager( );

		bool						LoadBsfFile(const char* pFilename);

		bool						PointInside(float fX, float fY);
		bool						RandomPoint(float& fX, float& fY);

		std::string					GetCurrentError( )	{ return m_strCurrentError; }

private:
		bool						PointInShape(SBoundaryShape& sShape, float fX, float fY);

		std::vector<SBoundaryShape>	m_vBoundaries;
		std::string					m_strCurrentError;
};
