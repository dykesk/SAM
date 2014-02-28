#ifndef __case_h
#define __case_h

#include <vector>

#include "object.h"
#include "variables.h"
#include "equations.h"

// case events allow the user interface to be updated
// when something internal in the case changes that needs to be reflected
// to the user, i.e. variables are recalculated...

class Case;
class ConfigInfo;

class CaseEvent
{
private:
	int m_type;
	wxArrayString m_vars;
	wxString m_str, m_str2;
public:
	enum { VARS_CHANGED, CONFIG_CHANGED };

	CaseEvent( int type ) : m_type(type) { }
	CaseEvent( int type, const wxString &str ) : m_type(type), m_str(str) { }
	CaseEvent( int type, const wxString &str1, const wxString &str2 ) : m_type(type), m_str(str1), m_str2(str2) { }
	CaseEvent( int type, const wxArrayString &vars ) : m_type(type), m_vars(vars) { }

	int GetType() { return m_type; }
	wxString GetString() { return m_str; }
	wxString GetString2() { return m_str2; }
	wxArrayString &GetVars() { return m_vars; }
};

class CaseEventListener
{
public:
	virtual void OnCaseEvent( Case *, CaseEvent & ) = 0;
};

class Case : public Object
{
public:
	Case();
	virtual ~Case();

	virtual Object *Duplicate();
	virtual bool Copy( Object *obj );
	virtual wxString GetTypeName();
	virtual void Write( wxOutputStream & );
	virtual bool Read( wxInputStream & );
	
	void SetConfiguration( const wxString &tech, const wxString &fin );
	void GetConfiguration( wxString *tech, wxString *fin );	
	ConfigInfo *GetConfiguration() { return m_config; }
	VarTable &Values() { return m_vals; }
	VarInfoLookup &Variables();
	EqnFastLookup &Equations();
	wxString GetTechnology() const;
	wxString GetFinancing() const;
	lk::env_t &CallbackEnvironment();
	lk::node_t *QueryCallback( const wxString &method, const wxString &object );
	
	// call this method when a variable is programmatically changed,
	// i.e. through a script callback, SamUL script, or other
	// indirect way of changing a variable.  causes any affected
	// variables to be recalculated, and updates any views
	void VariableChanged( const wxString &name );

	// recalculate any variables that are impacted by a changed value of 'trigger'
	// any views are updated with the variables that are consequently updated with 
	// new values as a result of the calculations
	// Note: if the 'trigger' variable is a library item (has the VF_LIBRARY flag)
	// this will also apply all the library values and cause any affected variables
	// to be subsequently updated
	int Recalculate( const wxString &trigger ); 

	// recalculate all equations in this case
	// CaseEvent is issued for all updated variables
	int RecalculateAll();

	VarTable &BaseCase();

	StringHash &Properties() { return m_properties; }
	wxString GetProperty( const wxString &id );
	void SetProperty( const wxString &id, const wxString &value );
	StringHash &Notes() { return m_notes; }
	wxString RetrieveNote( const wxString &id );
	void SaveNote( const wxString &id, const wxString &text );

	void AddListener( CaseEventListener *cel );
	void RemoveListener( CaseEventListener *cel );
	void ClearListeners();
	void SendEvent( CaseEvent e );

private:
	/* case data */
	ConfigInfo *m_config;
	VarTable m_vals;
	lk::env_t m_cbEnv;
	VarTable m_baseCase;
	StringHash m_properties;
	StringHash m_notes;
	std::vector<CaseEventListener*> m_listeners;
};

class CaseEvaluator : public EqnEvaluator
{
	Case *m_case;
public:	
	CaseEvaluator( Case *cc, VarTable &vars, EqnFastLookup &efl );
	virtual void SetupEnvironment( lk::env_t &env );	
	virtual int CalculateAll();
	virtual int Changed( const wxArrayString &vars );
	virtual int Changed( const wxString &trigger );

	bool UpdateLibrary( const wxString &trigger, wxArrayString &changed );
};

#endif