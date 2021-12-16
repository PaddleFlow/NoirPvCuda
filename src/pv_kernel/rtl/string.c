#include <pvdef.h>
#include <stdarg.h>
#include <pvbdk.h>

ULONG RtlGetStringLengthA(IN PSTR String,IN ULONG Limit)
{
	for(ULONG i=0;i<Limit;i++)
		if(String[i]=='\0')
			return i;
	return Limit;
}

ULONG RtlCopyStringA(OUT PSTR Destination,IN PSTR Source,IN ULONG DestinationLimit)
{
	for(ULONG i=0;i<DestinationLimit;i++)
	{
		Destination[i]=Source[i];
		if(Source[i]=='\0')return i;
	}
	return DestinationLimit;
}

ULONG RtlReverseStringA(IN OUT PSTR String,IN ULONG Limit)
{
	ULONG Length=RtlGetStringLengthA(String,Limit);
	if(Length<2)return Length;
	for(ULONG i=0;i<(Length>>1);i++)
	{
		char Left=String[i];
		String[i]=String[Length-i-1];
		String[Length-i-1]=Left;
	}
	return Length;
}

// FIXME: Implement a 2-Pass formatter for better string formatting.
ULONG RtlVPrintStringA(OUT PSTR Destination,IN ULONG Limit,IN PSTR Format,va_list ArgList)
{
	ULONG i=0,j=0;
	RtlZeroMemory(Destination,Limit);
	for(;Format[i];i++)
	{
		// If not to be formatted, simply copy the character.
		if(Format[i]!='%')
			Destination[j++]=Format[i];
		else
		{
			// Just a simple 1-Pass formatter.
			ULONG Increment=1;
			switch(Format[i+1])
			{
				case 'c':
				{
					char c=va_arg(ArgList,char);
					Destination[j]=c;
					i++;
					break;
				}
				case 'd':
				{
					char s[12];
					LONG d=va_arg(ArgList,LONG);
					BOOL n=d<0;
					ULONG k=0;
					for(;d;k++)
					{
						LONG r=d%10;
						d/=10;
						s[k]='0'+r;
					}
					s[k]=s[k+1]='\0';
					if(n)s[k]='-';
					RtlReverseStringA(s,sizeof(s));
					Increment=RtlCopyStringA(&Destination[j],s,Limit-j);
					i++;
					break;
				}
				case 'p':
				{
					char s[17];
					ULONG64 p=va_arg(ArgList,ULONG64);
					ULONG k=0;
					RtlFillMemory(s,'0',16);
					s[16]='\0';
					for(k=15;p;k--)
					{
						BYTE r=(BYTE)(p&0xf);
						p>>=4;
						s[k]=r<10?'0'+r:'A'+r-10;
					}
					Increment=RtlCopyStringA(&Destination[j],s,Limit-j);
					i++;
					break;
				}
				case 's':
				{
					PSTR s=va_arg(ArgList,PSTR);
					Increment=RtlCopyStringA(&Destination[j],s,Limit-j);
					i++;
					break;
				}
				case 'u':
				{
					char s[12];
					ULONG d=va_arg(ArgList,ULONG);
					ULONG k=0;
					for(;d;k++)
					{
						ULONG r=d%10;
						d/=10;
						s[k]='0'+r;
					}
					s[k]='\0';
					RtlReverseStringA(s,sizeof(s));
					Increment=RtlCopyStringA(&Destination[j],s,Limit-j);
					i++;
					break;
				}
				case 'x':
				{
					ULONG x=va_arg(ArgList,ULONG);
					char s[12];
					ULONG k=0;
					for(;x;k++)
					{
						ULONG r=x%0xf;
						x>>=4;
						s[k]=r<10?'0'+r:'a'+r-10;
					}
					s[k]='\0';
					RtlReverseStringA(s,sizeof(s));
					Increment=RtlCopyStringA(&Destination[j],s,Limit-j);
					i++;
					break;
				}
				case 'X':
				{
					ULONG x=va_arg(ArgList,ULONG);
					char s[12];
					ULONG k=0;
					for(;x;k++)
					{
						ULONG r=x%0xf;
						x>>=4;
						s[k]=r<10?'0'+r:'A'+r-10;
					}
					s[k]='\0';
					RtlReverseStringA(s,sizeof(s));
					Increment=RtlCopyStringA(&Destination[j],s,Limit-j);
					i++;
					break;
				}
				case '%':
				{
					Destination[j]='%';
					i++;
					break;
				}
				default:
				{
					Destination[j]='%';
					Destination[j+1]=Format[i+1];
					Increment=2;
					break;
				}
			}
			j+=Increment;
		}
		if(j>Limit)
		{
			j=Limit-1;
			break;
		}
	}
	return j+1;
}