/***************************************************************************************
 *
 * Memory leak demonstration file.
 *
 **************************************************************************************/

#include <windows.h>
#include <cstdio>
#include <list>
#include <vector>

#define DOCUMENT_DEFAULT_LINE_SIZE 10

// Generic document class
class Document
{
  public:
    Document(void) :
      total_lines(0),
      doc_lines()
    {
    }

    ~Document(void)
    {
      std::vector<char*>::iterator i;
      for( i = doc_lines.begin() ; i != doc_lines.end() ; ++i )
      {
	delete (*i);
      }
    }

    // how many lines in the document?
    size_t lines(void)
    {
      return total_lines;
    }//lines

    // Insert text into the document
    virtual void insert( const char* text )
    {
      char* p = new char[DOCUMENT_DEFAULT_LINE_SIZE];
      if( strlen(text) > DOCUMENT_DEFAULT_LINE_SIZE )
      {
	p = new char[strlen(text) + 1];  // copy line
      }
      strncpy(p,text,strlen(text)+1);
      doc_lines.insert(doc_lines.begin(),p);
      total_lines++;
    }

    void Display(void)
    {
      std::vector<char*>::iterator i;
      for( i = doc_lines.begin() ; i != doc_lines.end() ; ++i )
      {
	fprintf(stdout, "%s\n", *i);
      }
    }

  protected:
    size_t total_lines;  // total lines in the document
    std::vector<char*> doc_lines;

};// class Document

// PlainText document class
class DocumentPlainText : public Document
{
  public:
    DocumentPlainText()
    {
      encoding_table = new char[256];
    }
    ~DocumentPlainText()
    {
      delete encoding_table;
    }

    char map_char( char ch )
    {
      return encoding_table[ch];
    }

  private:
    char* encoding_table;
};// class DocumentPlainText

// HTML document class
class DocumentHTML : public Document
{
  public:
    DocumentHTML()
    {
      html_tags = new int[32];  // support 32 tags
    }
    ~DocumentHTML()
    {
      delete html_tags;
    }

    // Insert text into the document
    virtual void insert( const char* text )
    {
      char* html = new char[strlen(text) + 1 + 9]; // 'text' + <br></br>
      sprintf(html,"<br>%s</br>",text);            // format line
      Document::insert(html);                      // insert line
    }

  private:
    int* html_tags;
};// class DocumentPlainText

Document* DocumentFactory( int i )
{
  if( i % 2 )
  {
    return new DocumentPlainText();
  }
  return new DocumentHTML();
}//DocumentFactor

void DocumentDestructor( void* doc )
{
  delete doc;
}//DocumentDestructor

int main(void)
{
  // simulate user activity in a document
  for( ;; )
  {
    int x = rand() % 1000;
    Document* p = DocumentFactory(x); // create a document

    // insert line of text into the document
    for( int i = rand() % 100 ; i > 0 ; i-- )
    {
      char temp[128];
      sprintf(temp,"Some random number %d and %d\n", x, i); 
      p->insert(temp);
    }
    fprintf(stdout, "%d lines\n", p->lines());
    DocumentDestructor(p);
    Sleep(200);
  }
  return 0;
}// main
