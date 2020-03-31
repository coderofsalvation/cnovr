#include <cnovrtcc.h>
#include <cnovrparts.h>
#include <cnovrfocus.h>
#include <cnovrcanvas.h>
#include <cnovr.h>
#include <cnovrutil.h>
#include <stdlib.h>
#include <string.h>

cnovr_shader * rendermodelshader;
cnovr_pose    eightiessunpose;
cnovr_model * eightiessun;


const char * identifier;
cnovr_shader * shader;
cnovr_shader * shaderLines;
cnovr_shader * shaderBasic;
//cnovr_texture * texture;

//Making an array of chairs that is 2 sides.  6 chairs on a side. 10 rows deep.
#define CHAIRSCALE .95
#define MAX_CHAIRS 120
//cnovr_model * spinner_m[MAX_SPINNERS];
cnovr_model * chair[MAX_CHAIRS];

cnovr_model * isosphere;
cnovr_pose    isospherepose;
cnovrfocus_capture isocapture;
//cnovr_simple_node * spinner_n[MAX_SPINNERS];
int shutting_down;
cnovrfocus_capture focusblock[MAX_CHAIRS];

cnovr_canvas * canvas;

struct staticstore
{
	int initialized;
	cnovr_pose modelpose[MAX_CHAIRS];
	cnovr_pose targetpose[MAX_CHAIRS];
	float zapped[MAX_CHAIRS];
} * store;


int ExampleFocusEvent( int event, cnovrfocus_capture * cap, cnovrfocus_properties * prop, int buttoninfo )
{
	//printf( "EVENT: %d %d %d\n", event, cap->opaque, buttoninfo );
	cnovr_model * m = (cnovr_model*)cap->opaque;
	int id = m->iOpaque;
	switch( event )
	{
		case CNOVRF_DOWNNOFOCUS:
			if( buttoninfo == 0 ) { store->zapped[id] = .999; return 0; } //Catpured event
			break;
		case CNOVRF_LOSTFOCUS:
			CNOVRNamedPtrSave( "examplecodestore" );
			break;
		case CNOVRF_ACQUIREDFOCUS:
			store->zapped[id] = 1;
			break;
	}

	CNOVRGeneralHandleFocusEvent( m->focuscontrol, m->pose, prop, event, buttoninfo, CTRLA_PINCHBTN );

	if( event == CNOVRF_DRAG )
	{
		cnovr_pose * dragout = &store->modelpose[id];
		if(0)
		if( dragout->Rot[0] > .95 || dragout->Rot[0] < -.95 )
		{
			quatidentity( dragout->Rot );
			dragout->Pos[0] = floorf( dragout->Pos[0] * 10. ) / 10.;
			dragout->Pos[1] = floorf( dragout->Pos[1] * 10. ) / 10.;
			dragout->Pos[2] = floorf( dragout->Pos[2] * 10. ) / 10.;
		}
	}

	return 0;
}



int FocusEvent( int event, cnovrfocus_capture * cap, cnovrfocus_properties * prop, int buttoninfo );

void init( const char * identifier )
{
	ovrprintf( "Example init %s\n", identifier );
}

void UpdateFunction( void * tag, void * opaquev )
{
	static double start;
	double now = OGGetAbsoluteTime();
	if( start < 1 ) start = now;

	int i;
	double truedt = now - start;
	for (i = 0; i < MAX_CHAIRS; i++ )
	{
		double dt =  truedt*.2*1 - i * 3.14159;
		double ang = (i * .4);// + (now - start)*.2*1;
		cnovr_pose * pose = &store->modelpose[i];
		cnovr_pose * targ = &store->targetpose[i];
		float * zap = &store->zapped[i];

		if( *zap >= 1.0 ) { continue; } //spinner_n[i]->pose.Scale = .4;
		if( *zap > 0 ) *zap -= .001;
		if( *zap < 0 ) *zap = 0;
		float z = *zap;

		pose->Pos[0] = cnovr_lerp( targ->Pos[0], pose->Pos[0], z );
		pose->Pos[1] = cnovr_lerp( targ->Pos[1], pose->Pos[1], z );
		pose->Pos[2] = cnovr_lerp( targ->Pos[2], pose->Pos[2], z );
		//pose->Scale = cnovr_lerp( pose->Scale, .2, .01 );

		quatslerp( pose->Rot, targ->Rot, pose->Rot, z );
		
		
	}

	CNOVRCanvasClearFrame( canvas );
	static int frameno;
	frameno++;
	CNOVRCanvasSetLineWidth( canvas, 2 );
	CNOVRCanvasDrawText( canvas, 10, 10, trprintf( "%d", frameno ), 8 );
//	CNOVRCanvasTackSegment( canvas, 10, 10, 100, 10 );
	CNOVRCanvasSwapBuffers( canvas );

	return;
}


void RenderFunction( void * tag, void * opaquev )
{
	int i;
//	CNOVRRender( shader );
//	CNOVRRender( texture );
	CNOVRRender( rendermodelshader );
	CNOVRRender( eightiessun );


	CNOVRRender( shaderLines );
	for( i = 0; i < MAX_CHAIRS; i++ )
	{
		CNOVRRender( chair[i] );
	}

//	CNOVRRender( shaderBasic );
	CNOVRRender( isosphere );

	CNOVRRender( canvas );
}


static void example_scene_setup( void * tag, void * opaquev )
{
	printf( "+++ Example scene setup\n" );
	int i;
	shader = CNOVRShaderCreate( "assets/example" );
	shaderBasic = CNOVRShaderCreate( "assets/basic" );
	
	
	shaderLines = CNOVRShaderCreateWithPrefix( "fakelines", "#define OPAQUIFY" );
/*
	cnovr_simple_node * root = cnovrstate->pRootNode;
	node = CNOVRNodeCreateSimple( 1 );
	model = CNOVRModelCreate( 0, GL_TRIANGLES );
	CNOVRModelAppendCube( model, (cnovr_point3d){ 1.f, 1.f, 1.f }, 0, 0 );
	CNOVRNodeAddObject( node, shader );
	CNOVRNodeAddObject( node, model );
	CNOVRNodeAddObject( root, node );
*/
	canvas = CNOVRCanvasCreate( "ExampleCanvas", 128, 96, 0 );

	srand( 0 );
	for( i = 0; i < MAX_CHAIRS; i++ )
	{
		//spinner_n[i] = CNOVRNodeCreateSimple( 1 );
		chair[i] = CNOVRModelCreate( 0, GL_TRIANGLES );
//		cnovr_point4d extra = { (rand()%256)/255.0, (rand()%256)/255.0, (rand()%256)/255.0, 0 };
//		CNOVRModelAppendCube( spinner_m[i], (cnovr_point3d){ 1.f, 1.f, 1.f }, 0, &extra );
		CNOVRModelLoadFromFileAsync( chair[i], "chair.obj:barytc" );
		chair[i]->pose = &store->modelpose[i];
		chair[i]->iOpaque = i;
		focusblock[i].opaque = chair[i];
		focusblock[i].cb = ExampleFocusEvent;
		CNOVRModelSetInteractable( chair[i], &focusblock[i] );
	}

	isosphere = CNOVRModelCreate( 0, GL_TRIANGLES );
	isosphere->pose = &isospherepose;
	pose_make_identity( &isospherepose );
	CNOVRModelLoadFromFileAsync( isosphere, "isosphere.obj:barytc" ); //:barytc
	isocapture.tag = 0;
	isocapture.opaque = isosphere;
	isocapture.cb = CNOVRFocusDefaultFocusEvent;
	CNOVRModelSetInteractable( isosphere, &isocapture );
	
	
	rendermodelshader = CNOVRShaderCreate( "assets/rendermodelnearestaa" );

	eightiessun = CNOVRModelCreate( 0, GL_TRIANGLES );
	cnovr_point3d eightiessunsize = { 1, 1, 1 };
	CNOVRModelAppendMesh( eightiessun, 2, 2, 1, eightiessunsize ,0, 0 );
	pose_make_identity( &eightiessunpose );
	eightiessunpose.Pos[2] = -102;
	eightiessunpose.Pos[1] = 17;
	eightiessunpose.Scale = 10;
	eightiessun->pose = &eightiessunpose;
	CNOVRModelApplyTextureFromFileAsync( eightiessun, "80sSun.png" );


//	texture = CNOVRTextureCreate( 0, 0, 0 ); //Set to all 0 to have the load control these details.
//	CNOVRTextureLoadFileAsync( texture, "test.png" );

	UpdateFunction(0,0);
	CNOVRListAdd( cnovrLUpdate, 0, UpdateFunction );
	CNOVRListAdd( cnovrLRender2, 0, RenderFunction );
	//CNOVRListAdd( cnovrLCollide, 0, CollideFunction );
	printf( "+++ Example scene setup complete\n" );
}


void cyber_chair_stackerstart( const char * identifier )
{

	store = CNOVRNamedPtrData( "cyber_chair_stacker_store_v2", 0, sizeof( *store ) + 1024 );
	printf( "=== Initializing %p\n", store );
//	store->initialized = 0;
	if( !store->initialized || 1 )
	{
		int i;
		for( i = 0; i < MAX_CHAIRS; i++ )
		{
			pose_make_identity( &store->modelpose[i] );
			pose_make_identity( &store->targetpose[i] );
			int row = i / 8;
			int col = (i % 8);
			if( col >= 4 ) col += 1;
			col -= 4;
			store->targetpose[i].Pos[0] = row - 4.;
			store->targetpose[i].Pos[2] = (col)*.6;			
			store->zapped[i] = 0;
			store->targetpose[i].Scale = CHAIRSCALE;
			store->modelpose[i].Scale = CHAIRSCALE;
		}
		store->initialized = 1;
	}
	
	identifier = strdup(identifier);
	CNOVRJobTack( cnovrQPrerender, example_scene_setup, 0, 0, 0 );
	printf( "=== Cyber Chair Stacker start %s(%p) + %p %p\n", identifier, identifier );
}

void cyber_chair_stackerstop( const char * identifier )
{
	shutting_down = 1;
	//OGCancelThread( thdmax );
	printf( "=== End Cyber Chair Stacker stop\n" );
}


