#pragma once


namespace Antares {


	using namespace System;
	using namespace System::Text;

	public ref class CommandLine
	{
	public:


		String^ str; // Input string
		array<String^>^ tokens;

		String^ the_command;
		bool command_dispatched;

		String^ cmd_param1;
		String^ cmd_param2;
		bool e;
		bool showgui;
		bool showgui_specified;
		bool exit_on_completion;
		bool no_prompt;

		void error(String ^ str)
		{
			Console::WriteLine(str);

		}

		void set_defaults(void)
		{
			this->e = false;
			this->command_dispatched = false;
		    this->the_command="";
			this->cmd_param1="";
			this->cmd_param2="";
			this->showgui=true;
			this->showgui_specified=false;
			this->no_prompt = false;
			this->exit_on_completion=false;

		}
		void analyse(void)
		{

			int nt = this->tokens->Length;
		    int ind=1;
			while(ind<nt)
			{
				String^ tok = this->tokens[ind];

				if (tok=="cp" || tok=="mv") tok="-"+tok;

				if (tok=="-cp" || tok=="-mv" || tok=="/cp" || tok=="/mv")

				{
				   String^ cmd = tok->Substring(1,2);
				   if (the_command->Length>0)
				   {
					   this->error("ERROR: The commands "+the_command+" and "+cmd+" can't be used at the same time.");
					   return;
				   }
				   this->the_command = cmd;


				   if (ind + 2 >= nt)
				   {
					   this->error("ERROR: The command "+the_command+" requires both a source and destination.");
					   return;
				   }
				   ind++;
				   this->cmd_param1 = this->tokens[ind];
				   ind++;
				   this->cmd_param2 = this->tokens[ind];
				   ind++;
				   continue;
				}

				if (tok=="-g" || tok=="/g")
				{
					this->showgui_specified=true;
					ind++;
					continue;
				}

				this->error("ERROR: Unknown command line option: "+tok);
				return;
			}

			if (this->the_command->Length>0)
			{
				this->showgui= this->showgui_specified;
				this->no_prompt = true;
				this->exit_on_completion = !this->showgui;
			}
			



		}


		void parse(void)
		{
			StringBuilder^ sb = gcnew StringBuilder(str);
			int L = str->Length;
			bool escaped = false;
			int i2=0;
			for (int i=0; i<L; i++)
			{
				if (!escaped)
				{
					if (str[i]==' ')
					{
					    sb[i2]='\n';
						i2++;
						continue;
					}
					if (str[i]=='"')
					{
						escaped = true;
						continue;

					}
				}

				if (escaped && sb[i]=='"') 
				{
					escaped=false;
					continue;
				}
				sb[i2]=str[i];
				i2++;

			}
			sb->Length = i2;
            String ^str2 = sb->ToString();
			array<wchar_t>^ sep = {'\n'};
			tokens = str2->Split(sep,StringSplitOptions::RemoveEmptyEntries)  ;

		}



		CommandLine(String ^ inp_str)
		{

			this->str = inp_str;
			this->parse();
			this->set_defaults();
			this->analyse();

			//for each (String^ x in tokens)
			//	printf("Tok: $%s$\n",x);


		}

	};


}
#ifdef sadkhfs

/// <summary>
/// Arguments class
/// </summary>
public class Arguments
{ 
	/// <summary>
	/// Splits the command line. When main(string[] args) is used escaped quotes (ie a path "c:\folder\")
	/// Will consume all the following command line arguments as the one argument. 
	/// This function ignores escaped quotes making handling paths much easier.
	/// </summary>
	/// <param name="commandLine">The command line.</param>
	/// <returns></returns>
	public static string[] SplitCommandLine(string commandLine)
	{
		var translatedArguments = new StringBuilder(commandLine);
		var escaped = false;
		for (var i = 0; i < translatedArguments.Length; i++)
		{
			if (translatedArguments[i] == '"')
			{
				escaped = !escaped;
			}
			if (translatedArguments[i] == ' ' && !escaped)
			{
				translatedArguments[i] = '\n';
			}
		}

		var toReturn = translatedArguments.ToString().Split(new[] { '\n' }, StringSplitOptions.RemoveEmptyEntries);
		for (var i = 0; i < toReturn.Length; i++)
		{
			toReturn[i] = RemoveMatchingQuotes(toReturn[i]);
		}
		return toReturn;
	}

	public static string RemoveMatchingQuotes(string stringToTrim)
	{
		var firstQuoteIndex = stringToTrim.IndexOf('"');
		var lastQuoteIndex = stringToTrim.LastIndexOf('"');
		while (firstQuoteIndex != lastQuoteIndex)
		{
			stringToTrim = stringToTrim.Remove(firstQuoteIndex, 1);
			stringToTrim = stringToTrim.Remove(lastQuoteIndex - 1, 1); //-1 because we've shifted the indicies left by one
			firstQuoteIndex = stringToTrim.IndexOf('"');
			lastQuoteIndex = stringToTrim.LastIndexOf('"');
		}

		return stringToTrim;
	}

	private readonly Dictionary<string, Collection<string>> _parameters;
	private string _waitingParameter;

	public Arguments(IEnumerable<string> arguments)
	{
		_parameters = new Dictionary<string, Collection<string>>();

		string[] parts;

		//Splits on beginning of arguments ( - and -- and / )
		//And on assignment operators ( = and : )
		var argumentSplitter = new Regex(@"^-{1,2}|^/|=|:",
			RegexOptions.IgnoreCase | RegexOptions.Compiled);

		foreach (var argument in arguments)
		{
			parts = argumentSplitter.Split(argument, 3);
			switch (parts.Length)
			{
			case 1:
				AddValueToWaitingArgument(parts[0]);
				break;
			case 2:
				AddWaitingArgumentAsFlag();

				//Because of the split index 0 will be a empty string
				_waitingParameter = parts[1];
				break;
			case 3:
				AddWaitingArgumentAsFlag();

				//Because of the split index 0 will be a empty string
				string valuesWithoutQuotes = RemoveMatchingQuotes(parts[2]);

				AddListValues(parts[1], valuesWithoutQuotes.Split(','));
				break;
			}
		}

		AddWaitingArgumentAsFlag();
	}

	private void AddListValues(string argument, IEnumerable<string> values)
	{
		foreach (var listValue in values)
		{
			Add(argument, listValue);
		}
	}

	private void AddWaitingArgumentAsFlag()
	{
		if (_waitingParameter == null) return;

		AddSingle(_waitingParameter, "true");
		_waitingParameter = null;
	}

	private void AddValueToWaitingArgument(string value)
	{
		if (_waitingParameter == null) return;

		value = RemoveMatchingQuotes(value);

		Add(_waitingParameter, value);
		_waitingParameter = null;
	}

	/// <summary>
	/// Gets the count.
	/// </summary>
	/// <value>The count.</value>
	public int Count
	{
		get
		{
			return _parameters.Count;
		}
	}

	/// <summary>
	/// Adds the specified argument.
	/// </summary>
	/// <param name="argument">The argument.</param>
	/// <param name="value">The value.</param>
	public void Add(string argument, string value)
	{
		if (!_parameters.ContainsKey(argument))
			_parameters.Add(argument, new Collection<string>());

		_parameters[argument].Add(value);
	}

	public void AddSingle(string argument, string value)
	{
		if (!_parameters.ContainsKey(argument))
			_parameters.Add(argument, new Collection<string>());
		else
			throw new ArgumentException(string.Format("Argument {0} has already been defined", argument));

		_parameters[argument].Add(value);
	}

	public void Remove(string argument)
	{
		if (_parameters.ContainsKey(argument))
			_parameters.Remove(argument);
	}

	/// <summary>
	/// Determines whether the specified argument is true.
	/// </summary>
	/// <param name="argument">The argument.</param>
	/// <returns>
	/// <c>true</c> if the specified argument is true; otherwise, <c>false</c>.
	/// </returns>
	public bool IsTrue(string argument)
	{
		AssertSingle(argument);

		var arg = this[argument];

		return arg != null && arg[0].Equals("true", StringComparison.OrdinalIgnoreCase);
	}

	private void AssertSingle(string argument)
	{
		if (this[argument] != null && this[argument].Count > 1)
			throw new ArgumentException(string.Format("{0} has been specified more than once, expecting single value", argument));
	}

	public string Single(string argument)
	{
		AssertSingle(argument);

		//only return value if its NOT true, there is only a single item for that argument
		//and the argument is defined
		if (this[argument] != null && !IsTrue(argument))
			return this[argument][0];

		return null;
	}

	public bool Exists(string argument)
	{
		return (this[argument] != null && this[argument].Count > 0);
	}

	/// <summary>
	/// Gets the <see cref="System.Collections.ObjectModel.Collection&lt;T&gt;"/> with the specified parameter.
	/// </summary>
	/// <value></value>
	public Collection<string> this[string parameter]
	{
		get
		{
			return _parameters.ContainsKey(parameter) ? _parameters[parameter] : null;
		}
	}
}

#endif