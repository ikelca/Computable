/*
 * VTParse - an implementation of Paul Williams' DEC compatible state machine parser
 *
 * Author: Joshua Haberman <joshua@reverberate.org>
 *
 * This code is in the public domain.
 */

#include "vtparse.h"

void vtparse_init(vtparse_t *parser, vtparse_callback_t cb)
{
    parser->state                  = VTPARSE_STATE_GROUND;
    parser->num_intermediate_chars = 0;
    parser->num_params             = 0;
    parser->ignore_flagged         = 0;
    parser->cb                     = cb;
}

static void do_action(vtparse_t *parser, vtparse_action_t action, char ch)
{
    /* Some actions we handle internally (like parsing parameters), others
     * we hand to our client for processing */

    switch(action) {
        case VTPARSE_ACTION_PRINT:
        case VTPARSE_ACTION_EXECUTE:
        case VTPARSE_ACTION_HOOK:
        case VTPARSE_ACTION_PUT:
        case VTPARSE_ACTION_OSC_START:
        case VTPARSE_ACTION_OSC_PUT:
        case VTPARSE_ACTION_OSC_END:
        case VTPARSE_ACTION_UNHOOK:
        case VTPARSE_ACTION_CSI_DISPATCH:
        case VTPARSE_ACTION_ESC_DISPATCH:
            parser->cb(parser, action, ch);
            break;

        case VTPARSE_ACTION_IGNORE:
            /* do nothing */
            break;

        case VTPARSE_ACTION_COLLECT:
        {
            /* Append the character to the intermediate params */
            if(parser->num_intermediate_chars + 1 > MAX_INTERMEDIATE_CHARS)
                parser->ignore_flagged = 1;
            else
                parser->intermediate_chars[parser->num_intermediate_chars++] = ch;

            break;
        }

        case VTPARSE_ACTION_PARAM:
        {
            /* process the param character */
            if(ch == ';')
            {
                parser->num_params += 1;
                parser->params[parser->num_params-1] = 0;
            }
            else
            {
                /* the character is a digit */
                int current_param;

                if(parser->num_params == 0)
                {
                    parser->num_params = 1;
                    parser->params[0]  = 0;
                }

                current_param = parser->num_params - 1;
                parser->params[current_param] *= 10;
                parser->params[current_param] += (ch - '0');
            }

            break;
        }

        case VTPARSE_ACTION_CLEAR:
            parser->num_intermediate_chars = 0;
            parser->num_params            = 0;
            parser->ignore_flagged        = 0;
            break;

        default:
            parser->cb(parser, VTPARSE_ACTION_ERROR, 0);
    }
}

static void do_state_change(vtparse_t *parser, state_change_t change, char ch)
{
    /* A state change is an action and/or a new state to transition to. */

    vtparse_state_t  new_state = STATE(change);
    vtparse_action_t action    = ACTION(change);


    if(new_state)
    {
        /* Perform up to three actions:
         *   1. the exit action of the old state
         *   2. the action associated with the transition
         *   3. the entry action of the new state
         */

        vtparse_action_t exit_action = EXIT_ACTIONS[parser->state-1];
        vtparse_action_t entry_action = ENTRY_ACTIONS[new_state-1];

        if(exit_action)
            do_action(parser, exit_action, 0);

        if(action)
            do_action(parser, action, ch);

        if(entry_action)
            do_action(parser, entry_action, 0);

        parser->state = new_state;
    }
    else
    {
        do_action(parser, action, ch);
    }
}

void vtparse(vtparse_t *parser, unsigned char *data, int len)
{
    int i;
    for(i = 0; i < len; i++)
    {
        unsigned char ch = data[i];
        state_change_t change = STATE_TABLE[parser->state-1][ch];
        do_state_change(parser, change, ch);
    }
}

