-- snippetd sample config
-- Each snippet{} call stages one entry. `expand` is either a string
-- (literal) or a function (callback, evaluated at expansion time).

------------------------------------------------------------------- literals

snippet{ trigger = "btw",  expand = "by the way" }
snippet{ trigger = "omw",  expand = "on my way" }
snippet{ trigger = "@@",   expand = "pano@example.com" }

-- LaTeX; the $0 marker is not yet parsed, so it will insert literally for now
snippet{ trigger = "\\fr",   expand = "\\frac{$0}{}" }
snippet{ trigger = "\\frac", expand = "\\frac{}{}" }   -- longest-match test vs \fr

-- multiline: exercises the commit_string path later
snippet{
  trigger = ";sig",
  expand = [[
--
Pano
Department of Physics]],
}

-- shared suffix with "btw" reversed (w,t,b) vs (w,t,a) -- exercises the trie
snippet{ trigger = "atw", expand = "at the weekend" }

------------------------------------------------------------------ callbacks

snippet{
  trigger = ";date",
  expand = function() return os.date("%Y-%m-%d") end,
}

snippet{
  trigger = ";time",
  expand = function() return os.date("%H:%M") end,
}

snippet{
  trigger = ";count",
  expand = function()
    local n = 0
    for _ = 1, 10 do n = n + 1 end
    return "counted to " .. n
  end,
}

--------------------------------------------------------------- generated

-- config is a program: snippets can be built in a loop
local greek = { alpha = "α", beta = "β", gamma = "γ", delta = "δ" }
for name, glyph in pairs(greek) do
  snippet{ trigger = ";" .. name, expand = glyph }   -- multi-byte UTF-8 test
end

------------------------------------------------------- deliberate failures
-- Uncomment one at a time to check the validator's line-numbered errors.

-- snippet{ trigger = 42,   expand = "nope" }         -- trigger not a string
-- snippet{ trigger = "",   expand = "nope" }         -- empty trigger
-- snippet{ trigger = "x",  expand = { 1, 2 } }       -- expand wrong type
-- snippet{ trigger = "x" }                           -- missing expand
-- snippet("not a table")                             -- wrong arg type
